#include "Managers/AI/Hug/HugAI.hpp"

#include "Config/Config.hpp"

#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/HugShrink.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Utils/Actions/ActionUtils.hpp"

using namespace GTS;

namespace {

	constexpr float MINIMUM_HUG_DISTANCE = 110.0f;
	constexpr float GRAB_ANGLE = 70.0f;
	void RecordSneakingState(Actor* a_Performer, Actor* a_Prey) {
		bool Crawling = AnimationVars::Crawl::IsCrawling(a_Performer);
		bool Sneaking = a_Performer->IsSneaking();

		AnimationVars::Tiny::SetIsBeingCrawlHugged(a_Prey, Crawling);
		AnimationVars::Tiny::SetIsBeingSneakHugged(a_Prey, Sneaking);
	}

	bool ActorStateCheck(Actor* a_Actor) {
		bool Jumping = AnimationVars::Other::IsJumping(a_Actor);
		bool Ragdolled = IsRagdolled(a_Actor);
		bool Busy = AnimationVars::General::IsGTSBusy(a_Actor);
		return Jumping || Ragdolled || Busy;
	}

	bool CanHug(Actor* a_Performer, Actor* a_Prey) {

		if (a_Performer == a_Prey) {
			return false;
		}

		if (a_Prey->IsDead() || GetAV(a_Prey, ActorValue::kHealth) < 0.0f) {
			return false;
		}

		if (AnimationVars::General::IsTransitioning(a_Performer) || IsBeingHeld(a_Performer, a_Prey)) {
			return false;
		}

		if (ActorStateCheck(a_Performer) || ActorStateCheck(a_Prey)) {
			return false;
		}

		const float PredatorScale = get_visual_scale(a_Performer);
		// No need to check for BB scale in this case

		float SizeDifference = get_scale_difference(a_Performer, a_Prey, SizeType::VisualScale, false, true);


		float MinDist = MINIMUM_HUG_DISTANCE;
		constexpr float MinScale = Action_Hug;

		if (a_Performer->IsSneaking()) {
			if (AnimationVars::Crawl::IsCrawling(a_Performer)) {
				MinDist *= 2.35f;
			}
			else {
				MinDist *= 1.6f;
			}
		}

		const float PreyDistance = (a_Performer->GetPosition() - a_Prey->GetPosition()).Length();

		if (PreyDistance <= MinDist * PredatorScale) {

			if (SizeDifference > MinScale) {

				if (!a_Prey->IsPlayerRef() && !CanPerformActionOn(a_Performer, a_Prey, true)) {
					return false;
				}

				if (!IsHuman(a_Prey)) { // Allow hugs with humanoids only
					return false;
				}

				return SizeDifference <= GetHugShrinkThreshold(a_Performer);
			}
		}
		return false;
	}

	//--------------
	// HUG TASK
	//--------------

	bool HugAI_CanHugCrush(Actor* a_Performer, Actor* a_Prey) {

		const float HealthPercentage = GetHealthPercentage(a_Prey);
		const float StaminaPercentage = GetStaminaPercentage(a_Performer);
		const float HPCrushThreshold = GetHugCrushThreshold(a_Performer, a_Prey, true);
		const bool HasLowHP = HealthPercentage <= HPCrushThreshold;
		const bool StaminaCheck = Runtime::HasPerkTeam(a_Performer, Runtime::PERK.GTSPerkHugMightyCuddles) && StaminaPercentage >= 0.75f;
		const auto& Settings = Config::AI.Hugs;
		const bool Teammate = IsTeammate(a_Prey) || a_Prey->IsPlayerRef();
		const bool Hostile = IsHostile(a_Performer, a_Prey) || IsHostile(a_Prey, a_Performer);
		const bool CanStartCrush = Config::AI.Hugs.fKillProb > 0.01f;

		const bool Killable =
			(Settings.bKillFollowersOrPlayer && Teammate) || //Teammate/Player Check
			((Settings.bKillFriendlies || Hostile) && !Teammate); //If Kill Friendly is enabled or Tiny is hostile And Not a teammate Allow

		return (HasLowHP || StaminaCheck) && Killable && CanStartCrush;
	}


	bool HugAI_CanShrink(Actor* a_Performer, Actor* a_Prey) {
		const float SizeDiff = get_scale_difference(a_Performer, a_Prey, SizeType::TargetScale, false, true);
		const bool TooSmall = SizeDiff >= GetHugShrinkThreshold(a_Performer);
		const bool CanStartShrink = Config::AI.Hugs.fShrinkProb > 0.01f;

		return !TooSmall && CanStartShrink;
	}

	bool HugAI_CanHeal(Actor* a_Performer, Actor* a_Prey) {
		const bool Teammate = IsTeammate(a_Performer) && (IsTeammate(a_Prey) || a_Prey->IsPlayerRef());
		const bool HasPerk = Runtime::HasPerkTeam(a_Performer, Runtime::PERK.GTSPerkHugsLovingEmbrace);
		const bool Hostile = IsHostile(a_Performer, a_Prey) || IsHostile(a_Prey, a_Performer);
		const bool CanStartHeal = Config::AI.Hugs.fHealProb > 0.01f;
		
		return HasPerk && !Hostile && Teammate && CanStartHeal;
	}

	bool HugAI_ShouldStop(Actor* a_Performer, Actor* a_Prey) {
		const bool Teammate = IsTeammate(a_Performer) && (IsTeammate(a_Prey) || a_Prey->IsPlayerRef());
		const bool CanCrush = HugAI_CanHugCrush(a_Performer, a_Prey);
		const bool CanHeal = HugAI_CanHeal(a_Performer, a_Prey);
		const bool CanShrink = HugAI_CanShrink(a_Performer, a_Prey) || (!Config::AI.Hugs.bStopIfCantShrink && Teammate);

		return !(CanCrush || CanHeal || CanShrink);
	}

	void HugAI_StartLogicTask(Actor* a_Performer, Actor* a_Prey) {


		const std::string TaskName = std::format("HugAI_{}", a_Performer->formID);
		const ActorHandle PerformerHandle = a_Performer->CreateRefHandle();
		const ActorHandle PreyHandle = a_Prey->CreateRefHandle();

		TaskManager::Run(TaskName, [=](auto& progressData) {

			if (!State::Live()) return false;

			const auto& Settings = Config::AI.Hugs;

			if (!PerformerHandle || !PreyHandle) {
				return false;
			}

			Actor* PerformerActor = PerformerHandle.get().get();
			Actor* PreyActor = PreyHandle.get().get();

			if (!PerformerActor || !PreyActor) {
				return false;
			}

			const auto& TransientData = Transient::GetActorData(PerformerActor);
			if (!TransientData) {
				return false;
			}
			TransientData->ActionTimer.UpdateDelta(Config::AI.Hugs.fInterval);

			const bool IsDead = PreyActor->IsDead() || PerformerActor->IsDead();
			const bool IsBusy = AnimationVars::Hug::IsHugCrushing(PerformerActor) || AnimationVars::Hug::IsHugHealing(PerformerActor);
			const bool GentleAnim = IsTeammate(PreyActor) || PreyActor->IsPlayerRef();

			if (!HugShrink::GetHuggiesActor(PerformerActor) || IsRagdolled(PerformerActor)) {
				if (!GentleAnim) {
					PushForward(PerformerActor, PreyActor, 300.0f);
				}
				return false;
			}

			//Should we stop? Means we can't do any other actions on the Tiny
			const bool ShouldStop = HugAI_ShouldStop(PerformerActor, PreyActor);

			if (TransientData->ActionTimer.ShouldRun() && !IsBusy && !IsDead && !ShouldStop) {

				UpdateFriendlyHugs(PerformerActor, PreyActor, !GentleAnim);
				const bool Teammate = (IsTeammate(PerformerActor) && (IsTeammate(PreyActor) || PreyActor->IsPlayerRef()));
				//Reduce if folllower or player

				const float ShrinkProbability = Teammate ? Settings.fFriendlyShrinkProb : Settings.fShrinkProb;
				const int SelectedAction = RandomIntWeighted({
					HugAI_CanHeal(PerformerActor,PreyActor) ? static_cast<int>(Settings.fHealProb) : 0,
					HugAI_CanShrink(PerformerActor,PreyActor) ? static_cast<int>(ShrinkProbability) : 0,
					HugAI_CanHugCrush(PerformerActor,PreyActor) ? static_cast<int>(Settings.fKillProb) : 0,
					100 //Base Chance to do nothing
				});


				switch (SelectedAction) {
					case 0:{ //HEAL
						StartHealingAnimation(PerformerActor, PreyActor);
						break;
					}

					case 1:{ //SHRINK
						AnimationManager::StartAnim("Huggies_Shrink", PerformerActor);
						AnimationManager::StartAnim("Huggies_Shrink_Victim", PreyActor);
						break;
					}

					case 2:{ //CRUSH
						AnimationManager::StartAnim("Huggies_HugCrush", PerformerActor);
						AnimationManager::StartAnim("Huggies_HugCrush_Victim", PreyActor);
						break;
					}

					default: {}
				}

			}

			if (ShouldStop) {
				UpdateFriendlyHugs(PerformerActor, PreyActor, !GentleAnim);
				AbortHugAnimation(PerformerActor, PreyActor);
			}

			if (IsDead) {
				return false;
			}

			return true;
		});
	}

}

namespace GTS {

	std::vector<Actor*> HugAI_FilterList(Actor* a_Performer, const std::vector<Actor*>& a_PotentialPrey) {
		// Get vore target for actor

		if (!a_Performer) {
			return {};
		}

		if (AnimationVars::General::IsGTSBusy(a_Performer)) {
			return {};
		}

		if (GetStaminaPercentage(a_Performer) < 0.25f) {
			return {};
		}

		if (!AnimationVars::General::CanDoPaired(a_Performer) && !AnimationVars::Other::IsSynched(a_Performer) && !AnimationVars::Grab::HasGrabbedTiny(a_Performer)) {
			return {};
		}

		const auto CharacterController = a_Performer->GetCharController();
		if (!CharacterController) {
			return {};
		}

		return SelectTargetsInFront(a_Performer, a_PotentialPrey, a_PotentialPrey.size(), GRAB_ANGLE, true, [a_Performer](auto prey) {
			return CanHug(a_Performer, prey);
		});
	}

	void HugAI_Start(Actor* a_Performer, Actor* a_Prey) {

		RecordSneakingState(a_Performer, a_Prey); // Needed to determine which hugs to play: sneak or crawl ones (when sneaking)
		HugShrink::HugActor(a_Performer, a_Prey);

		AnimationManager::StartAnim("Huggies_Try", a_Performer);

		if (a_Performer->IsSneaking()) {
			AnimationManager::StartAnim("Huggies_Try_Victim_S", a_Prey); // GTSBEH_HugAbsorbStart_Sneak_V
		}
		else {
			AnimationManager::StartAnim("Huggies_Try_Victim", a_Prey); //   GTSBEH_HugAbsorbStart_V
		}
		DisarmActor(a_Performer);
		DisarmActor(a_Prey);
		HugAI_StartLogicTask(a_Performer, a_Prey);
	}
}

