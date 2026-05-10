#include <cmath>

#include "Managers/MaxSizeManager.hpp"
#include "Config/Config.hpp"
#include "Constants.hpp"
#include "Systems/Misc/Time.hpp"
#include "Utils/Actions/ButtCrushUtils.hpp"

using namespace GTS;

namespace SizeOverride {

	constexpr float SIZE_OVERRIDE_MIN = 0.05f;
	constexpr float ACTION_FIT_RESULT_MIN = 0.051f;
	constexpr float ACTION_FIT_RESULT_MAX = 1.0f;
	constexpr float ACTION_FIT_STRICTEST_THRESHOLD = Action_Vore + 0.01f;
	constexpr float ACTION_FIT_UPDATE_INTERVAL = 0.15f;
	constexpr float ACTION_FIT_SCALE_THRESHOLD = 0.01f;

	struct ActionFitCache {
		float PlayerVisualScale = 1.0f;
		float Limit = ACTION_FIT_RESULT_MAX;
		double RefreshAfter = 0.0;
		bool Initialized = false;
	};

	ActionFitCache& GetActionFitCache() {
		static ActionFitCache cache;
		return cache;
	}

	bool IsImportantTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && !GTS::IsTeammate(a_actor) && a_actor->IsEssential();
	}

	bool IsHostileTarget(Actor* a_actor) {
		if (!a_actor || a_actor->IsPlayerRef() || GTS::IsTeammate(a_actor)) {
			return false;
		}

		Actor* player = PlayerCharacter::GetSingleton();
		if (!player) {
			return false;
		}

		if (GTS::IsHostile(player, a_actor) || GTS::IsHostile(a_actor, player)) {
			return true;
		}

		Actor* actorCombatTarget = a_actor->GetActorRuntimeData().currentCombatTarget.get().get();
		Actor* playerCombatTarget = player->GetActorRuntimeData().currentCombatTarget.get().get();
		return actorCombatTarget == player || playerCombatTarget == a_actor;
	}

	float ComputeActionFitLimit(float a_playerScale) {
		float playerScale = std::max(a_playerScale, ACTION_FIT_RESULT_MIN);
		if (playerScale >= ACTION_FIT_STRICTEST_THRESHOLD) {
			return ACTION_FIT_RESULT_MAX;
		}

		float actionLimit = playerScale / ACTION_FIT_STRICTEST_THRESHOLD;
		return std::clamp(actionLimit, ACTION_FIT_RESULT_MIN, ACTION_FIT_RESULT_MAX);
	}

	float GetActionFitLimit(bool a_forceRefresh = false) {
		Actor* player = PlayerCharacter::GetSingleton();
		if (!player) {
			return ACTION_FIT_RESULT_MAX;
		}

		auto& cache = GetActionFitCache();
		const float playerScale = get_visual_scale(player);
		const double now = Time::WorldTimeElapsed();
		const bool playerScaleChanged = std::abs(playerScale - cache.PlayerVisualScale) >= ACTION_FIT_SCALE_THRESHOLD;
		const bool refreshExpired = now >= cache.RefreshAfter;

		if (a_forceRefresh || !cache.Initialized || playerScaleChanged || refreshExpired) {
			cache.PlayerVisualScale = playerScale;
			cache.Limit = ComputeActionFitLimit(playerScale);
			cache.RefreshAfter = now + ACTION_FIT_UPDATE_INTERVAL;
			cache.Initialized = true;
		}

		return cache.Limit;
	}

	float GetConfiguredOverrideForActor(Actor* a_actor) {
		if (!a_actor) {
			return 0.0f;
		}

		if (a_actor->IsPlayerRef()) {
			return Config::Balance.fMaxPlayerSizeOverride;
		}

		if (GTS::IsTeammate(a_actor)) {
			if (Config::Balance.bFollowerDynamicActionFit) {
				return GetActionFitLimit();
			}
			return Config::Balance.fMaxFollowerSize;
		}

		if (IsHostileTarget(a_actor)) {
			if (Config::Balance.bHostileDynamicActionFit) {
				return GetActionFitLimit();
			}

			if (Config::Balance.fMaxHostileSize > SIZE_OVERRIDE_MIN) {
				return Config::Balance.fMaxHostileSize;
			}
		}

		if (IsImportantTarget(a_actor)) {
			if (Config::Balance.bImportantDynamicActionFit) {
				return GetActionFitLimit();
			}

			if (Config::Balance.fMaxImportantSize > SIZE_OVERRIDE_MIN) {
				return Config::Balance.fMaxImportantSize;
			}
		}

		if (Config::Balance.bOtherDynamicActionFit) {
			return GetActionFitLimit();
		}

		return Config::Balance.fMaxOtherSize;
	}

	bool SizeOverrideEnabled() {
		return !Config::Balance.bBalanceMode && Persistent::UnlockMaxSizeSliders.value;
	}

	void MassMode_ApplySizeOverride(Actor* a_actor, float& GetLimit) {
		if (SizeOverrideEnabled()) {
			float overrideLimit = GetConfiguredOverrideForActor(a_actor);

			if (overrideLimit > SIZE_OVERRIDE_MIN) {
				GetLimit = overrideLimit;
			}
		}
	}
}

namespace {

    constexpr float DEFAULT_MAX = 1'000'000.0f;
	constexpr float SIZE_OVERRIDE_RESTORE_MAX = 1.0f;
	constexpr float SIZE_OVERRIDE_RESTORE_EPS = 0.0001f;

    bool IsSizeUnlocked() {
		// Reports true when player has ColossalGrowth perk and used gts unlimited command, else it's false
		if (Persistent::UnlockMaxSizeSliders.value) {
			const bool Unlocked = Runtime::HasPerk(PlayerCharacter::GetSingleton(), Runtime::PERK.GTSPerkColossalGrowth);
			return Unlocked;
		}
		return false;
	}

	void RecordOverkillSize_Transient(TransientActorData* Data, float value, float kills) {
		if (Data) {
			Data->CollossalGrowthSizeBonus = value;
			Data->OverkillSizeBonus = kills;
		}
	}

    //Ported From Papyrus
	float GetSizeFromPerks(RE::Actor* a_Actor) {
		float BonusSize = 0.0f;

		if (Runtime::HasPerk(a_Actor, Runtime::PERK.GTSPerkSizeManipulation3)) {
			BonusSize += static_cast<float>(a_Actor->GetLevel()) * Perk_SizeManipulation_3;
		}

		if (Runtime::HasPerk(a_Actor, Runtime::PERK.GTSPerkSizeManipulation2)) {


			BonusSize += GetGtsSkillLevel(a_Actor) * Perk_SizeManipulation_2;

		}

		if (Runtime::HasPerk(a_Actor, Runtime::PERK.GTSPerkSizeManipulation1)) {
			BonusSize += Perk_SizeManipulation_1;
		}

		return BonusSize;
	}

    float get_endless_height(Actor* actor) {
		float endless = 0.0f;

		if (Runtime::HasPerk(actor, Runtime::PERK.GTSPerkColossalGrowth) && Persistent::UnlockMaxSizeSliders.value) {
			endless = DEFAULT_MAX;
		}

		return endless;
	}

	void UpdateSizeOverrideRestore(Actor* actor, float overrideLimit, bool overrideActive) {
		if (!actor) {
			return;
		}

		auto persistent = Persistent::GetActorData(actor);
		auto transient = Transient::GetActorData(actor);

		if (!persistent || !transient) {
			return;
		}

		if (!overrideActive) {
			transient->SizeOverrideLastRawLimit = 0.0f;
			transient->SizeOverrideRestoreTargetScale = 0.0f;
			transient->SizeOverrideRestoreActive = false;
			return;
		}

		const float currentTargetScale = get_target_scale(actor);
		const bool limitIncreased = transient->SizeOverrideLastRawLimit > SizeOverride::SIZE_OVERRIDE_MIN &&
			overrideLimit > transient->SizeOverrideLastRawLimit + SIZE_OVERRIDE_RESTORE_EPS;

		// Remember the pre-clamp target, but only restore once when the configured limit grows.
		if (currentTargetScale > overrideLimit + SIZE_OVERRIDE_RESTORE_EPS) {
			const float restoreTarget = std::min(currentTargetScale, SIZE_OVERRIDE_RESTORE_MAX);
			if (restoreTarget > overrideLimit + SIZE_OVERRIDE_RESTORE_EPS) {
				transient->SizeOverrideRestoreTargetScale = std::max(transient->SizeOverrideRestoreTargetScale, restoreTarget);
				transient->SizeOverrideRestoreActive = true;
			}
		}

		if (limitIncreased && transient->SizeOverrideRestoreActive) {
			const float restoredTarget = std::min(
				std::min(transient->SizeOverrideRestoreTargetScale, overrideLimit),
				SIZE_OVERRIDE_RESTORE_MAX
			);

			if (restoredTarget > currentTargetScale + SIZE_OVERRIDE_RESTORE_EPS) {
				set_target_scale(actor, restoredTarget);
			}

			if (restoredTarget + SIZE_OVERRIDE_RESTORE_EPS >= transient->SizeOverrideRestoreTargetScale ||
				overrideLimit + SIZE_OVERRIDE_RESTORE_EPS >= SIZE_OVERRIDE_RESTORE_MAX) {
				transient->SizeOverrideRestoreTargetScale = 0.0f;
				transient->SizeOverrideRestoreActive = false;
			}
		}

		transient->SizeOverrideLastRawLimit = overrideLimit;
	}



    float get_mass_based_limit(Actor* actor, float NaturalScale) { // gets mass based size limit for Player if using Mass Based mode

		float GetLimit = 1.0f;

		if (auto data = Persistent::GetActorData(actor); data) {
			float ExtraMagicSize = data->fExtraPotionMaxScale * MassMode_ElixirPowerMultiplier;
			float MaxSize = data->fSizeLimit; // Cap max size through normal size rules
			float size_calc = NaturalScale + data->fMassBasedLimit + ExtraMagicSize;

			// Multiplying MassBasedSizeLimit by Natural Scale is a bad idea, it messes up max scaling with level 100 perk, displays 32x out of 34x

			GetLimit = std::clamp(size_calc, NaturalScale, MaxSize);
			SizeOverride::MassMode_ApplySizeOverride(actor, GetLimit);
		}

        return GetLimit;
    }

	float get_default_size_limit(float NaturalScale, float BaseLimit) { // default size limit for everyone
		float size_calc = NaturalScale + ((BaseLimit - 1.0f) * NaturalScale);
		float GetLimit = std::clamp(size_calc, 0.1f, DEFAULT_MAX);

		return GetLimit;
	}

    /*float get_follower_size_limit(float NaturalScale, float FollowerLimit) { // Self explanatory
        float size_calc = NaturalScale + ((FollowerLimit) * NaturalScale);
        float GetLimit = std::clamp(size_calc, 1.0f * FollowerLimit, DEFAULT_MAX);

        return GetLimit;
    }

    float get_npc_size_limit(float NaturalScale, float NPCLimit) { // get non-follower size limit
        float size_calc = NaturalScale + ((NPCLimit - 1.0f) * NaturalScale);
		float GetLimit = std::clamp(size_calc, 1.0f * NPCLimit, DEFAULT_MAX);

        return GetLimit;
    }*/
}

namespace GTS {

	float GetActionCompatibleSizeLimit(bool a_forceRefresh) {
		return SizeOverride::GetActionFitLimit(a_forceRefresh);
	}

    void UpdateMaxScale(Actor* a_actor) {
       	GTS_PROFILE_SCOPE("MaxSizeManager: UpdateMaxScale");

		const bool IsMassBased = Config::Balance.sSizeMode == "kMassBased"; // Should DLL use mass based formula for Player?

		float Limit = 1.0f;
		// -------------------------------------------------------------------------------------------------

		if (auto data = Persistent::GetActorData(a_actor); data) {
			Limit = data->fSizeLimit; // Cap max size through normal size rules
		}

		// Apply custom limits only if player has Perk and gts unlimited command was executed, else use GlobalLimit

        float Endless = get_endless_height(a_actor);

        const float NaturalScale = get_natural_scale(a_actor, true);
		float GetLimit = get_default_size_limit(NaturalScale, Limit); // Default size limit
		
		if (IsMassBased) {
			GetLimit = get_mass_based_limit(a_actor, NaturalScale); // Apply Player Mass-Based max size
		}

		/*else if (QuestStage > 100 && FollowerLimit > 0.0f && FollowerLimit != 1.0f && !a_actor->IsPlayerRef() && IsTeammate(a_actor)) {
			GetLimit = get_follower_size_limit(NaturalScale, FollowerLimit); // Apply Follower Max Size
		}
		else if (QuestStage > 100 && NPCLimit > 0.0f && NPCLimit != 1.0f && !a_actor->IsPlayerRef() && !IsTeammate(a_actor)) {
            GetLimit = get_npc_size_limit(NaturalScale, NPCLimit); // Apply Other NPC's max size
		}*/

		float TotalLimit = GetLimit;
		float OverrideLimit = 0.0f;
		bool HasOverrideLimit = false;

		if (IsSizeUnlocked()) {
			OverrideLimit = SizeOverride::GetConfiguredOverrideForActor(a_actor);

			if (OverrideLimit > SizeOverride::SIZE_OVERRIDE_MIN) {
				TotalLimit = OverrideLimit;
				HasOverrideLimit = true;
			}
		}

		Ench_Potions_ApplyBonuses(a_actor, TotalLimit); // Apply after size override, else Butt Crush growth won't be able to surpass size limit for example

		if (get_max_scale(a_actor) < TotalLimit + Endless || get_max_scale(a_actor) > TotalLimit + Endless) {
			set_max_scale(a_actor, TotalLimit);
		}

		UpdateSizeOverrideRestore(a_actor, OverrideLimit, HasOverrideLimit);
		
    }

    //Ported From Papyrus
	float GetExpectedMaxSize(RE::Actor* a_Actor, float start_value) {
		const bool IsMassBased = Config::Balance.sSizeMode == "kMassBased";
		const float LevelBonus = 1.0f + GetGtsSkillLevel(a_Actor) * 0.006f;
		float PotionSize = 0.0f;
		float Colossal_kills = 0.0f;
		float Colossal_lvl = 1.0f;
		float QuestMult = 0.6f;

		if (auto data = Persistent::GetActorData(a_Actor); data) {
			PotionSize = data->fExtraPotionMaxScale * (IsMassBased ? MassMode_ElixirPowerMultiplier : 1.0f);
		}

		const auto Quest = Runtime::GetQuest(Runtime::QUST.GTSQuestProgression);
		if (!Quest) {
			return 1.0f;
		}



		if (a_Actor->IsPlayerRef()) {
			//If Player
			const auto Stage = Quest->GetCurrentStageID();
			if (Stage < 20) {
				return 1.0f;
			}
			//Each stage after 20 adds 0.04f in steps of 10 stages
			//Base value + Current Stage - 20 / 10
			QuestMult = 0.10f + static_cast<float>(Stage - 20) / 10.f * 0.04f;
			if (Stage >= 80) QuestMult = 0.60f;
		}

		auto Transient = Transient::GetActorData(a_Actor);

			if (Runtime::HasPerk(a_Actor, Runtime::PERK.GTSPerkColossalGrowth)) { //Total Size Control Perk
			Colossal_lvl = 1.15f;
			if (auto KillData = Persistent::GetKillCountData(a_Actor)) {
				Colossal_kills = static_cast<float>(KillData->iTotalKills) * (0.02f / Characters_AssumedCharSize);
				if (Runtime::HasPerk(a_Actor, Runtime::PERK.GTSPerkOverindulgence)) {
					Colossal_lvl += KillData->iTotalKills * Overkills_BonusSizePerKill;
				}
			}

			RecordOverkillSize_Transient(Transient, Colossal_lvl, Colossal_kills);
			
			if (SizeOverride::SizeOverrideEnabled()) {
				float overrideLimit = SizeOverride::GetConfiguredOverrideForActor(a_Actor);

				if (overrideLimit > SizeOverride::SIZE_OVERRIDE_MIN) {
					return overrideLimit;
				}
			}
		} 
    	else {
			RecordOverkillSize_Transient(Transient, 1.0f, 0.0f);
		}

		const float MaxAllowedSize = start_value + (QuestMult + GetSizeFromPerks(a_Actor)) * LevelBonus;
		return (MaxAllowedSize + PotionSize + Colossal_kills) * Colossal_lvl;
	}

    //Ported From Papyrus
	void UpdateGlobalSizeLimit(Actor* a_actor) {
		if (const auto data = Persistent::GetActorData(a_actor)) {
			data->fSizeLimit = GetExpectedMaxSize(a_actor);
		}
	}

	void VisualScale_CheckForSizeAdjustment(Actor* actor, float& ScaleMult) {
		if (IsSizeUnlocked()) {
			const float SizeLimit = SizeOverride::GetConfiguredOverrideForActor(actor);
			if (SizeLimit > SizeOverride::SIZE_OVERRIDE_MIN) {
				const float NaturalScale = std::max(get_natural_scale(actor, true), 0.001f);
				ScaleMult = std::clamp(SizeLimit / NaturalScale, 0.0f, 1.0f);
			}
		}
	}

	void Ench_Potions_ApplyBonuses(Actor* actor, float& value) {
		if (actor) {
			value *= Potion_GetSizeMultiplier(actor); //Potion size
			value += GetButtCrushSize(actor); //Butt crush added size
			value *= 1.0f + Ench_Aspect_GetPower(actor); //Enchantment
		}
	}

	float MassMode_GetValuesForMenu(Actor* actor) {
		if (actor) {
			if (auto data = Persistent::GetActorData(actor); data) {
				float MassModeScale = data->fSizeLimit;
				Ench_Potions_ApplyBonuses(actor, MassModeScale);
				return MassModeScale;
			}
		}
		return 1.0f;
	}
}

