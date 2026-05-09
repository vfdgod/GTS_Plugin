#include "Managers/AI/Thigh/ThighSandwichAI.hpp"
#include "Config/Config.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Controllers/ThighSandwichController.hpp"
#include "Utils/Actions/ActionUtils.hpp"

using namespace GTS;

namespace {

	constexpr float MINIMUM_SANDWICH_DISTANCE = 70.0f;
	constexpr float SANDWICH_ANGLE = 60;
	constexpr bool ALLOW_DEAD = false;

	bool CanSandwich(Actor* a_Performer, Actor* a_Prey) {

		if (a_Performer == a_Prey) {
			return false;
		}

		if ((a_Prey->IsDead() || GetAV(a_Prey, ActorValue::kHealth) < 0.0f) && !ALLOW_DEAD) {
			return false;
		}

		if (IsBeingHeld(a_Performer, a_Prey)) {
			return false;
		}

		if (!CanPerformActionOn(a_Performer, a_Prey, false)) {
			return false;
		}

		const float PredScale = get_visual_scale(a_Performer);
		const float SizeDiff = get_scale_difference(a_Performer, a_Prey, SizeType::VisualScale, true, false);
		constexpr float MinimumScale = Action_Sandwich;
		constexpr float MinimumDistance = MINIMUM_SANDWICH_DISTANCE;

		float PreyDistance = (a_Performer->GetPosition() - a_Prey->GetPosition()).Length();

		if (PreyDistance <= (MinimumDistance * PredScale) && SizeDiff > MinimumScale) {
			return true;
		}

		return false;
	}

}

namespace GTS {

	std::vector<Actor*> ThighSandwichAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PotentialPrey) {

		if (!a_Performer) {
			return {};
		}

		if (AnimationVars::Crawl::IsCrawling(a_Performer)) {
			return {};
		}

		const auto CharacterController = a_Performer->GetCharController();
		if (!CharacterController) {
			return {};
		}

		auto preys = SelectTargetsInFront(a_Performer, a_PotentialPrey, a_PotentialPrey.size(), SANDWICH_ANGLE, true, [a_Performer](auto prey) {
			return CanSandwich(a_Performer, prey);
		});
		return GetMaxActionableTinyCount(a_Performer, preys);
	}


	//This gets called by SandwichingData::Update() in Thighsandwichcontroller.cpp
	void ThighSandwichAI_DecideAction(Actor* a_Performer, bool a_HavePrey) {

		if (a_HavePrey && GetPercentageAV(a_Performer,ActorValue::kStamina) > 0.05f) {

			const auto& Settings = Config::AI.ThighSandwich;

			switch (RandomIntWeighted({
					static_cast<int>(Settings.fProbabilityLight),
					static_cast<int>(Settings.fProbabilityHeavy),
					100
			})){

				case 0: {
					AnimationManager::StartAnim("ThighAttack", a_Performer);
					return;
				}

				case 1: {
					AnimationManager::StartAnim("ThighAttack_Heavy", a_Performer);
					return;
				}
				default:{}

			}
		}
		else {
			AnimationManager::StartAnim("ThighExit", a_Performer);
		}
	}

	void ThighSandwichAI_Start(Actor* a_Performer, const std::vector<Actor*>& a_PreyList) {
		for (const auto& prey : a_PreyList) {
			ThighSandwichController::StartSandwiching(a_Performer, prey, false);
			auto node = find_node(a_Performer, "GiantessRune", false);
			if (node) {
				node->local.scale = 0.01f;
				update_node(node);
			}
		}
	}
}
