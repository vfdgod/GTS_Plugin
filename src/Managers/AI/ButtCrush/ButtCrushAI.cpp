
#include "Managers/AI/ButtCrush/ButtCrushAI.hpp"
#include "Config/Config.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Grab.hpp"
#include "Managers/Animation/Controllers/ButtCrushController.hpp"
#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Utils/Actions/ActionUtils.hpp"
#include "Utils/Actions/ButtCrushUtils.hpp"

using namespace GTS;

namespace {

	constexpr float MINIMUM_BUTTCRUSH_DISTANCE = 95.0f;
	constexpr float BUTTCRUSH_ANGLE = 70;
	constexpr bool ALLOW_DEAD = false;

	bool CanButtCrush(Actor* a_Performer, Actor* a_Prey) {

		if (a_Performer == a_Prey) {
			return false;
		}

		if ((a_Prey->IsDead() || GetAV(a_Prey, ActorValue::kHealth) < 0.0f) && !ALLOW_DEAD) {
			return false;
		}

		if (!CanPerformActionOn(a_Performer, a_Prey, false)) {
			return false;
		}

		const float PredScale = get_visual_scale(a_Performer);
		const float SizeDiff = get_scale_difference(a_Performer, a_Prey, SizeType::VisualScale, true, false);

		float MINIMUM_BUTTCRUSH_SCALE = Action_ButtCrush;
		constexpr float MINIMUM_DISTANCE = MINIMUM_BUTTCRUSH_DISTANCE;

		if (AnimationVars::Crawl::IsCrawling(a_Performer)) {
			MINIMUM_BUTTCRUSH_SCALE *= 1.5f;
		}

		const float PreyDistance = (a_Performer->GetPosition() - a_Prey->GetPosition()).Length();

		if (PreyDistance <= (MINIMUM_DISTANCE * PredScale) && SizeDiff >= MINIMUM_BUTTCRUSH_SCALE) {
			return true;
		}

		return false;

	}

	void ButtCrushAI_StartLogicTask(Actor* a_Performer) {

		if (!a_Performer) return;

		const std::string TaskName = std::format("ButtCrushAI_{}", a_Performer->formID);

		const auto PerformerHandle = a_Performer->CreateRefHandle();
		const auto BeginTime = Time::WorldTimeElapsed();
		const auto& ActorTransient = Transient::GetActorData(a_Performer);

		TaskManager::Run(TaskName, [=](auto& progressData) {

			if (!PerformerHandle || !ActorTransient) {
				return false;
			}

			const auto FrameB = Time::WorldTimeElapsed() - BeginTime;
			if (FrameB <= 2.0f) {
				return true;
			}

			Actor* ActorRef = PerformerHandle.get().get();

			const bool CanGrow = ButtCrush_IsAbleToGrow(ActorRef, GetGrowthLimit(ActorRef));
			const bool BlockGrowth = IsActionOnCooldown(ActorRef, CooldownSource::Misc_AiGrowth);

			if (AnimationVars::Growth::IsChangingSize(ActorRef)) { // Growing/shrinking
				ApplyActionCooldown(ActorRef, CooldownSource::Misc_AiGrowth);
			}

			ActorTransient->ActionTimer.UpdateDelta(Config::AI.ButtCrush.fInterval);
			if (BlockGrowth && !ActorTransient->ActionTimer.ShouldRun()) {
				return true;
			}

			if (!AnimationVars::Growth::IsChangingSize(ActorRef)){
				//If we dont have the perk or for some reason the action needs to be canceled just play the attack anim immediatly
				if (!Runtime::HasPerkTeam(ActorRef, Runtime::PERK.GTSPerkButtCrushAug2) || !AnimationVars::ButtCrush::IsButtCrushing(ActorRef)) {
					AnimationManager::StartAnim("ButtCrush_Attack", ActorRef);
				}
				else if (CanGrow && RandomBool(Config::AI.ButtCrush.fGrowProb)) {
					ApplyActionCooldown(ActorRef, CooldownSource::Misc_AiGrowth);
					AnimationManager::StartAnim("ButtCrush_Growth", ActorRef);
				}
				// Can't grow any further or random bool tells us to stop
				else if (!CanGrow || RandomBool(Config::AI.ButtCrush.fCrushProb)) {
					AnimationManager::StartAnim("ButtCrush_Attack", ActorRef);
				}
			}

			if (!AnimationVars::ButtCrush::IsButtCrushing(ActorRef)) {
				return false; // End the task
			}

			return true;
		});
	}
}

namespace GTS {

	std::vector<Actor*> ButtCrushAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_ViablePreyList) {

		if (!a_Performer) {
			return {};
		}

		if (AnimationVars::General::IsGTSBusy(a_Performer) || AnimationVars::Growth::IsChangingSize(a_Performer)) {
			return {};
		}

		//Don't do action if we're holding an actor
		if (Grab::GetHeldActor(a_Performer)) {
			return {};
		}

		const auto CharController = a_Performer->GetCharController();
		if (!CharController) {
			return {};
		}

		auto preys = SelectTargetsInFront(a_Performer, a_ViablePreyList, a_ViablePreyList.size(), BUTTCRUSH_ANGLE, true, [a_Performer](auto prey) {
			return CanButtCrush(a_Performer, prey);
		});
		return GetMaxActionableTinyCount(a_Performer, preys);

	}

    void ButtCrushAI_Start(Actor* A_Performer, Actor* a_Prey) {

		if (RandomBool(Config::AI.ButtCrush.fButtCrushTypeProb) && Runtime::HasPerkTeam(A_Performer, Runtime::PERK.GTSPerkButtCrushAug1)) {
			ButtCrushController::StartButtCrush(A_Performer, a_Prey, false);
			ButtCrushAI_StartLogicTask(A_Performer);
		}
		else {
			AnimationManager::StartAnim("ButtCrush_StartFast", A_Performer);
		}
    }
}
