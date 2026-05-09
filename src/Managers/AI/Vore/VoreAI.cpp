#include "Managers/AI/Vore/VoreAI.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Controllers/VoreController.hpp"
#include "Utils/Actions/ActionUtils.hpp"

namespace {

	constexpr float BASE_CONE_WIDTH = 70.0f;
	constexpr float VORE_ANGLE = 75.0f;
	constexpr bool ALLOW_DEAD = false;

	constexpr float MINIMUM_VORE_SCALE = GTS::Action_Vore;
	constexpr float MINIMUM_DISTANCE = 95.0f;
}


namespace GTS {

	static bool VoreAI_CanVore(Actor* a_Pred, Actor* a_Prey) {

		if (a_Pred == a_Prey) {
			return false;
		}

		if ((a_Prey->IsDead() || GetAV(a_Prey, ActorValue::kHealth) < 0.0f) && !ALLOW_DEAD) {
			return false;
		}

		if (IsBeingHeld(a_Pred, a_Prey)) {
			return false;
		}

		const auto Transient = Transient::GetActorData(a_Prey);
		if (Transient) {
			if (Transient->CanBeVored == false) {
				return false;
			}
		}

		const float PredScale = get_visual_scale(a_Pred);
		const float SizeDiff = get_scale_difference(a_Pred, a_Prey, SizeType::VisualScale, true, false);
		const float PreyDistance = (a_Pred->GetPosition() - a_Prey->GetPosition()).Length();

		if (IsInsect(a_Prey, true) || IsBlacklisted(a_Prey)) {
			return false;
		}

		if (!IsLiving(a_Prey)) {
			return false;
		}

		if (!CanPerformActionOn(a_Pred,a_Prey,false)) {
			return false;
		}

		if (PreyDistance <= (MINIMUM_DISTANCE * PredScale) && SizeDiff > MINIMUM_VORE_SCALE) {
			return true;
		}

		return false;
	}

	std::vector<Actor*> VoreAI_FilterList(Actor* a_Pred, const std::vector<Actor*>& a_PotentialPrey) {

		// Get vore target for actor
		if (!a_Pred) {
			return {};
		}

		auto CharController = a_Pred->GetCharController();
		if (!CharController) {
			return {};
		}

		auto preys = SelectTargetsInFront(a_Pred, a_PotentialPrey, a_PotentialPrey.size(), VORE_ANGLE, true, [a_Pred](auto prey) {
			return VoreAI_CanVore(a_Pred, prey);
		});
		return GetMaxActionableTinyCount(a_Pred, preys);
	}

	void VoreAI_StartVore(Actor* a_Predator, const std::vector<Actor*>& a_PotentialPrey) {

		auto& VoreData = VoreController::GetSingleton().GetVoreData(a_Predator);
		for (auto Prey : a_PotentialPrey) {
			VoreData.AddTiny(Prey);
		}

		DamageAV(a_Predator, ActorValue::kStamina, 30 * a_PotentialPrey.size());
		AnimationManager::StartAnim("StartVore", a_Predator);
	}
}



