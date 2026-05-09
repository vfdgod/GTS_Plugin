#include "Managers/AI/Thigh/ThighCrushAI.hpp"
#include "Config/Config.hpp"
#include "Managers/HighHeel.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Utils/Actions/ActionUtils.hpp"

using namespace GTS;

namespace {

	constexpr float MINIMUM_THIGH_DISTANCE = 58.0f;
	constexpr float THIGH_ANGLE = 75;
	constexpr bool ALLOW_DEAD = false;


	bool CanThighCrush(Actor* a_Performer, Actor* a_Prey) {
		if (a_Performer == a_Prey) {
			return false;
		}
		if ((a_Prey->IsDead() || GetAV(a_Prey, ActorValue::kHealth) < 0.0f) && !ALLOW_DEAD) {
			return false;
		}
		if (AnimationVars::Crawl::IsCrawling(a_Performer) || AnimationVars::General::IsTransitioning(a_Performer) || IsBeingHeld(a_Performer, a_Prey)) {
			return false;
		}

		const float PredScale = get_visual_scale(a_Performer);
		const float SizeDiff = get_scale_difference(a_Performer, a_Prey, SizeType::VisualScale, false, true);
		const float MinimumDistance = MINIMUM_THIGH_DISTANCE + HighHeelManager::GetBaseHHOffset(a_Performer).Length();
		constexpr float MinimumThighCrushScale = Action_AI_ThighCrush;
		const float PreyDistance = (a_Performer->GetPosition() - a_Prey->GetPosition()).Length();

		if (PreyDistance <= MinimumDistance * PredScale) {

			if (SizeDiff > MinimumThighCrushScale) {

				if (CanPerformActionOn(a_Performer, a_Prey, false)) {
					return true;
				}
			}
		}
		return false;
	}

	void StartThighCrushTask(Actor* giant) {

		const std::string TaskName = std::format("ThighCrush_{}", giant->formID);

		const ActorHandle GiantHandle = giant->CreateRefHandle();
		const auto& ActorTransient = Transient::GetActorData(giant);
		const double StartTime = Time::WorldTimeElapsed();

		TaskManager::Run(TaskName, [=](auto& progressData) {

			if (!State::Live()) return false;

			const auto& ThighSettings = Config::AI.ThighCrush;

			if (!GiantHandle || !ActorTransient || !ThighSettings.bEnableAction) {
				return false;
			}

			Actor* ActorRef = GiantHandle.get().get();

			if (!ActorRef) {
				return false;
			}

			const double FinishTime = Time::WorldTimeElapsed();
			ActorTransient->ActionTimer.UpdateDelta(ThighSettings.fInterval);

			if (FinishTime - StartTime > 0.10) {

				//Are we in a thigh crush anim (idle or acting)
				if (!AnimationVars::Action::IsThighCrushing(ActorRef)) {
					return false;
				}

				if (ActorTransient->ActionTimer.ShouldRunFrame()) {

					bool StaminaTooLow = GetAV(ActorRef, ActorValue::kStamina) <= 2.0f;
					DamageAV(ActorRef, ActorValue::kStamina, 0.025f);
					std::vector<Actor*> ValidTargetsInFront = ThighCrushAI_FilterList(ActorRef, find_actors());

					if (StaminaTooLow) {
						AnimationManager::StartAnim("ThighLoopExit", ActorRef);
						return true;
					}
					if (ValidTargetsInFront.empty()) {
						AnimationManager::StartAnim("ThighLoopExit", ActorRef);
						return true;
					}

					//Only Heavy is used
					if (RandomBool(ThighSettings.fProbabilityHeavy)) {
						AnimationManager::StartAnim("ThighLoopAttack", ActorRef);
					}
				}
			}
			return true;
		});
	}
}

namespace GTS {

	//--------------------
	//------- THIGH CRUSH
	//--------------------

	std::vector<Actor*> ThighCrushAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PreyList) {

		if (!a_Performer) {
			return {};
		}

		auto charController = a_Performer->GetCharController();
		if (!charController) {
			return {};
		}

		return SelectTargetsInFront(a_Performer, a_PreyList, a_PreyList.size(), THIGH_ANGLE, true, [a_Performer](auto prey) {
			return CanThighCrush(a_Performer, prey);
		});
	}

	void ThighCrushAI_Start(Actor* a_Performer) {

		if (AnimationVars::Action::IsThighCrushing(a_Performer)) {
			return;
		}

		AnimationManager::StartAnim("ThighLoopEnter", a_Performer);

		StartThighCrushTask(a_Performer);
	}
}
