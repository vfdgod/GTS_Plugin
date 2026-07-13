#include <cmath>

#include "Managers/MaxSizeManager.hpp"
#include "Config/Config.hpp"
#include "Constants.hpp"
#include "Systems/Misc/Time.hpp"
#include "Utils/Actions/ButtCrushUtils.hpp"

using namespace GTS;

namespace SizeOverride {

	constexpr float SIZE_OVERRIDE_MIN = 0.05f;
	constexpr float SIZE_OVERRIDE_MAX = 1'000'000.0f;
	constexpr float SIZE_RULE_DEFAULT_FIXED_LIMIT = 1.0f;
	constexpr float ACTION_FIT_RESULT_MIN = 0.051f;
	constexpr float ACTION_FIT_RESULT_MAX = 1.0f;
	constexpr float ACTION_FIT_STRICTEST_THRESHOLD = Action_Vore + 0.01f;
	constexpr float ACTION_FIT_EXTRA_SHRINK_MAX = ACTION_FIT_RESULT_MAX - ACTION_FIT_RESULT_MIN;

	struct ResolvedSizeLimitRule {
		bool HasOverrideLimit = false;
		bool LockToNatural = false;
		float OverrideLimit = 0.0f;
	};

	struct ParsedSizeLimitRule {
		LSizeLimitRuleTarget_t Target = LSizeLimitRuleTarget_t::kHumanoidNPC;
		LSizeLimitRuleMode_t Mode = LSizeLimitRuleMode_t::kFixedLimit;
		float Value = SIZE_RULE_DEFAULT_FIXED_LIMIT;
		bool CombatOnly = false;
	};

	template <class Enum>
	std::string EnumName(Enum a_value) {
		return std::string(magic_enum::enum_name(a_value));
	}

	bool TryParseTarget(const std::string& a_value, LSizeLimitRuleTarget_t& a_out) {
		if (auto parsed = magic_enum::enum_cast<LSizeLimitRuleTarget_t>(a_value); parsed.has_value()) {
			a_out = *parsed;
			return true;
		}
		return false;
	}

	bool TryParseMode(const std::string& a_value, LSizeLimitRuleMode_t& a_out) {
		if (auto parsed = magic_enum::enum_cast<LSizeLimitRuleMode_t>(a_value); parsed.has_value()) {
			a_out = *parsed;
			return true;
		}
		return false;
	}

	bool IsModeAllowedForTarget(LSizeLimitRuleTarget_t a_target, LSizeLimitRuleMode_t a_mode) {
		return !(a_target == LSizeLimitRuleTarget_t::kPlayer && a_mode == LSizeLimitRuleMode_t::kActionFit);
	}

	bool RuleTargetSupportsCombatOnly(LSizeLimitRuleTarget_t a_target) {
		return a_target == LSizeLimitRuleTarget_t::kHostile;
	}

	void NormalizeSizeLimitRule(SizeLimitRule_t& a_rule) {
		LSizeLimitRuleTarget_t target = LSizeLimitRuleTarget_t::kHumanoidNPC;
		if (!TryParseTarget(a_rule.sTarget, target)) {
			target = LSizeLimitRuleTarget_t::kHumanoidNPC;
			a_rule.sTarget = EnumName(target);
		}

		LSizeLimitRuleMode_t mode = LSizeLimitRuleMode_t::kFixedLimit;
		if (!TryParseMode(a_rule.sMode, mode) || !IsModeAllowedForTarget(target, mode)) {
			mode = target == LSizeLimitRuleTarget_t::kPlayer ?
				LSizeLimitRuleMode_t::kSystemAuto :
				LSizeLimitRuleMode_t::kFixedLimit;
			a_rule.sMode = EnumName(mode);
		}

		if (!RuleTargetSupportsCombatOnly(target)) {
			a_rule.bCombatOnly = false;
		}

		if (mode == LSizeLimitRuleMode_t::kFixedLimit) {
			if (a_rule.fValue < SIZE_OVERRIDE_MIN) {
				a_rule.fValue = SIZE_RULE_DEFAULT_FIXED_LIMIT;
			}
			else if (a_rule.fValue > SIZE_OVERRIDE_MAX) {
				a_rule.fValue = SIZE_OVERRIDE_MAX;
			}
		}
	}

	void AppendRule(std::vector<SizeLimitRule_t>& a_rules, LSizeLimitRuleTarget_t a_target, LSizeLimitRuleMode_t a_mode, float a_value = SIZE_RULE_DEFAULT_FIXED_LIMIT) {
		SizeLimitRule_t rule;
		rule.bEnabled = true;
		rule.sTarget = EnumName(a_target);
		rule.sMode = EnumName(a_mode);
		rule.fValue = a_value;
		NormalizeSizeLimitRule(rule);
		a_rules.push_back(std::move(rule));
	}

	void AppendMigratedLegacyRule(std::vector<SizeLimitRule_t>& a_rules, LSizeLimitRuleTarget_t a_target, float a_scale, bool a_actionFit) {
		if (a_actionFit && IsModeAllowedForTarget(a_target, LSizeLimitRuleMode_t::kActionFit)) {
			AppendRule(a_rules, a_target, LSizeLimitRuleMode_t::kActionFit);
			return;
		}

		if (a_scale <= SIZE_OVERRIDE_MIN) {
			return;
		}

		if (a_scale >= SIZE_OVERRIDE_MAX - SIZE_OVERRIDE_MIN) {
			AppendRule(a_rules, a_target, LSizeLimitRuleMode_t::kUnlimited, SIZE_OVERRIDE_MAX);
			return;
		}

		AppendRule(a_rules, a_target, LSizeLimitRuleMode_t::kFixedLimit, a_scale);
	}

	void AppendMigratedLegacyOtherRules(std::vector<SizeLimitRule_t>& a_rules, float a_scale, bool a_actionFit) {
		constexpr LSizeLimitRuleTarget_t Targets[] = {
			LSizeLimitRuleTarget_t::kHumanoidNPC,
			LSizeLimitRuleTarget_t::kAnimal,
			LSizeLimitRuleTarget_t::kCreature,
			LSizeLimitRuleTarget_t::kDragon,
			LSizeLimitRuleTarget_t::kGiantMammoth,
			LSizeLimitRuleTarget_t::kMechanical,
		};

		for (auto target : Targets) {
			AppendMigratedLegacyRule(a_rules, target, a_scale, a_actionFit);
		}
	}

	bool HasLegacyRuleConfiguration() {
		const float otherScale = Config::Balance.fMaxOtherSize > SIZE_OVERRIDE_MIN ?
			Config::Balance.fMaxOtherSize :
			Config::Balance.fMaxOrdinaryNPCSize;

		return Config::Balance.fMaxPlayerSizeOverride > SIZE_OVERRIDE_MIN ||
			Config::Balance.fMaxFollowerSize > SIZE_OVERRIDE_MIN ||
			Config::Balance.fMaxHostileSize > SIZE_OVERRIDE_MIN ||
			Config::Balance.fMaxImportantSize > SIZE_OVERRIDE_MIN ||
			otherScale > SIZE_OVERRIDE_MIN ||
			Config::Balance.bFollowerDynamicActionFit ||
			Config::Balance.bHostileDynamicActionFit ||
			Config::Balance.bImportantDynamicActionFit ||
			Config::Balance.bOtherDynamicActionFit;
	}

	void EnsureInitialized() {
		auto& balance = Config::Balance;
		if (!balance.bSizeLimitRulesInitialized) {
			if (balance.SizeLimitRules.empty() && HasLegacyRuleConfiguration()) {
				AppendMigratedLegacyRule(balance.SizeLimitRules, LSizeLimitRuleTarget_t::kPlayer, balance.fMaxPlayerSizeOverride, false);
				AppendMigratedLegacyRule(balance.SizeLimitRules, LSizeLimitRuleTarget_t::kFollower, balance.fMaxFollowerSize, balance.bFollowerDynamicActionFit);
				AppendMigratedLegacyRule(balance.SizeLimitRules, LSizeLimitRuleTarget_t::kHostile, balance.fMaxHostileSize, balance.bHostileDynamicActionFit);
				AppendMigratedLegacyRule(balance.SizeLimitRules, LSizeLimitRuleTarget_t::kImportant, balance.fMaxImportantSize, balance.bImportantDynamicActionFit);

				const float otherScale = balance.fMaxOtherSize > SIZE_OVERRIDE_MIN ?
					balance.fMaxOtherSize :
					balance.fMaxOrdinaryNPCSize;
				AppendMigratedLegacyOtherRules(balance.SizeLimitRules, otherScale, balance.bOtherDynamicActionFit);
			}

			balance.bSizeLimitRulesInitialized = true;
		}

		for (auto& rule : balance.SizeLimitRules) {
			NormalizeSizeLimitRule(rule);
		}
	}

	const std::vector<ParsedSizeLimitRule>& GetParsedRules() {
		struct RuleCache {
			std::uint64_t Frame = std::numeric_limits<std::uint64_t>::max();
			std::vector<ParsedSizeLimitRule> Rules;
		};

		thread_local RuleCache cache;
		const auto currentFrame = Time::FramesElapsed();
		if (cache.Frame == currentFrame) {
			return cache.Rules;
		}

		EnsureInitialized();
		cache.Frame = currentFrame;
		cache.Rules.clear();
		cache.Rules.reserve(Config::Balance.SizeLimitRules.size());

		for (const auto& rule : Config::Balance.SizeLimitRules) {
			if (!rule.bEnabled) {
				continue;
			}

			LSizeLimitRuleTarget_t target = LSizeLimitRuleTarget_t::kHumanoidNPC;
			LSizeLimitRuleMode_t mode = LSizeLimitRuleMode_t::kFixedLimit;
			if (!TryParseTarget(rule.sTarget, target) ||
				!TryParseMode(rule.sMode, mode) ||
				!IsModeAllowedForTarget(target, mode)) {
				continue;
			}

			cache.Rules.push_back({
				.Target = target,
				.Mode = mode,
				.Value = rule.fValue,
				.CombatOnly = rule.bCombatOnly,
			});
		}

		return cache.Rules;
	}

	bool IsImportantTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && a_actor->IsEssential();
	}

	bool IsHostileTarget(Actor* a_actor) {
		if (!a_actor || a_actor->IsPlayerRef()) {
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

	float GetActionFitLimit() {
		Actor* player = PlayerCharacter::GetSingleton();
		if (!player) {
			return ACTION_FIT_RESULT_MAX;
		}

		const float baseLimit = ComputeActionFitLimit(get_visual_scale(player));
		const float extraShrink = std::clamp(Config::Balance.fActionFitExtraShrink, 0.0f, ACTION_FIT_EXTRA_SHRINK_MAX);
		return std::clamp(baseLimit - extraShrink, ACTION_FIT_RESULT_MIN, ACTION_FIT_RESULT_MAX);
	}

	bool IsAnimalTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && Runtime::HasKeyword(a_actor, Runtime::KYWD.AnimalKeyword);
	}

	bool IsCreatureTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && Runtime::HasKeyword(a_actor, Runtime::KYWD.CreatureKeyword);
	}

	bool IsHumanoidNPCTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && GTS::IsHuman(a_actor);
	}

	bool IsDragonTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && GTS::IsDragon(a_actor);
	}

	bool IsGiantMammothTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && (GTS::IsGiant(a_actor) || GTS::IsMammoth(a_actor));
	}

	bool IsMechanicalTarget(Actor* a_actor) {
		return a_actor && !a_actor->IsPlayerRef() && GTS::IsMechanical(a_actor);
	}

	bool ActorMatchesRuleTarget(Actor* a_actor, LSizeLimitRuleTarget_t a_target) {
		switch (a_target) {
			case LSizeLimitRuleTarget_t::kPlayer:
				return a_actor && a_actor->IsPlayerRef();
			case LSizeLimitRuleTarget_t::kFollower:
				return a_actor && GTS::IsTeammate(a_actor);
			case LSizeLimitRuleTarget_t::kHostile:
				return IsHostileTarget(a_actor);
			case LSizeLimitRuleTarget_t::kImportant:
				return IsImportantTarget(a_actor);
			case LSizeLimitRuleTarget_t::kHumanoidNPC:
				return IsHumanoidNPCTarget(a_actor);
			case LSizeLimitRuleTarget_t::kAnimal:
				return IsAnimalTarget(a_actor);
			case LSizeLimitRuleTarget_t::kCreature:
				return IsCreatureTarget(a_actor);
			case LSizeLimitRuleTarget_t::kDragon:
				return IsDragonTarget(a_actor);
			case LSizeLimitRuleTarget_t::kGiantMammoth:
				return IsGiantMammothTarget(a_actor);
			case LSizeLimitRuleTarget_t::kMechanical:
				return IsMechanicalTarget(a_actor);
			default:
				return false;
		}
	}

	ResolvedSizeLimitRule ResolveRuleForActor(Actor* a_actor) {
		ResolvedSizeLimitRule resolved;
		if (!a_actor) {
			return resolved;
		}

		for (const auto& rule : GetParsedRules()) {
			if (!ActorMatchesRuleTarget(a_actor, rule.Target)) {
				continue;
			}
			if (rule.CombatOnly && RuleTargetSupportsCombatOnly(rule.Target) && !a_actor->IsInCombat()) {
				continue;
			}

			switch (rule.Mode) {
				case LSizeLimitRuleMode_t::kNaturalCeiling:
					resolved.HasOverrideLimit = true;
					resolved.OverrideLimit = std::max(get_natural_scale(a_actor, true), SIZE_OVERRIDE_MIN);
					return resolved;
				case LSizeLimitRuleMode_t::kNaturalLock:
					resolved.HasOverrideLimit = true;
					resolved.LockToNatural = true;
					resolved.OverrideLimit = std::max(get_natural_scale(a_actor, true), SIZE_OVERRIDE_MIN);
					return resolved;
				case LSizeLimitRuleMode_t::kFixedLimit:
					resolved.HasOverrideLimit = true;
					resolved.OverrideLimit = std::clamp(rule.Value, SIZE_OVERRIDE_MIN, SIZE_OVERRIDE_MAX);
					return resolved;
				case LSizeLimitRuleMode_t::kActionFit:
					resolved.HasOverrideLimit = true;
					resolved.OverrideLimit = GetActionFitLimit();
					return resolved;
				case LSizeLimitRuleMode_t::kSystemAuto:
					return resolved;
				case LSizeLimitRuleMode_t::kUnlimited:
					resolved.HasOverrideLimit = true;
					resolved.OverrideLimit = SIZE_OVERRIDE_MAX;
					return resolved;
				default:
					return resolved;
			}
		}

		return resolved;
	}

	float GetConfiguredOverrideForActor(Actor* a_actor) {
		const auto resolved = ResolveRuleForActor(a_actor);
		return resolved.HasOverrideLimit ? resolved.OverrideLimit : 0.0f;
	}

	bool SizeOverrideEnabled() {
		return Config::Balance.bSizeLimitRulesEnabled && !Config::Balance.bBalanceMode;
	}

}

namespace {

    constexpr float DEFAULT_MAX = 1'000'000.0f;
	constexpr float SIZE_OVERRIDE_RESTORE_MAX = 1.0f;
	constexpr float SIZE_OVERRIDE_RESTORE_EPS = 0.0001f;

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

	void UpdateSizeOverrideRestore(Actor* actor, TransientActorData* transient, float overrideLimit, bool overrideActive) {
		if (!actor || !transient) {
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
	float get_mass_based_limit(PersistentActorData* data, float NaturalScale) { // gets mass based size limit for Player if using Mass Based mode

		float GetLimit = 1.0f;

		if (data) {
			float ExtraMagicSize = data->fExtraPotionMaxScale * MassMode_ElixirPowerMultiplier;
			const float MaxSize = std::max(data->fSizeLimit, NaturalScale); // std::clamp requires low <= high.
			float size_calc = NaturalScale + data->fMassBasedLimit + ExtraMagicSize;

			// Multiplying MassBasedSizeLimit by Natural Scale is a bad idea, it messes up max scaling with level 100 perk, displays 32x out of 34x

			GetLimit = std::clamp(size_calc, NaturalScale, MaxSize);
		}

        return GetLimit;
    }

	float ComputeExpectedMaxSize(Actor* a_actor, PersistentActorData* a_persistent, TransientActorData* a_transient) {
		if (!a_actor) {
			return 1.0f;
		}

		const bool IsMassBased = Config::Balance.sSizeMode == "kMassBased";
		const float LevelBonus = 1.0f + GetGtsSkillLevel(a_actor) * 0.006f;
		const float PotionSize = a_persistent ?
			a_persistent->fExtraPotionMaxScale * (IsMassBased ? MassMode_ElixirPowerMultiplier : 1.0f) :
			0.0f;
		float Colossal_kills = 0.0f;
		float Colossal_lvl = 1.0f;
		float QuestMult = 0.6f;

		const auto Quest = Runtime::GetQuest(Runtime::QUST.GTSQuestProgression);
		if (!Quest) {
			return 1.0f;
		}

		if (a_actor->IsPlayerRef()) {
			const auto Stage = Quest->GetCurrentStageID();
			if (Stage < 20) {
				return 1.0f;
			}

			// Each stage after 20 adds 0.04 in steps of 10 stages.
			QuestMult = 0.10f + static_cast<float>(Stage - 20) / 10.0f * 0.04f;
			if (Stage >= 80) {
				QuestMult = 0.60f;
			}
		}

		if (Runtime::HasPerk(a_actor, Runtime::PERK.GTSPerkColossalGrowth)) {
			Colossal_lvl = 1.15f;
			if (auto KillData = Persistent::GetKillCountData(a_actor)) {
				Colossal_kills = static_cast<float>(KillData->iTotalKills) * (0.02f / Characters_AssumedCharSize);
				if (Runtime::HasPerk(a_actor, Runtime::PERK.GTSPerkOverindulgence)) {
					Colossal_lvl += KillData->iTotalKills * Overkills_BonusSizePerKill;
				}
			}
			RecordOverkillSize_Transient(a_transient, Colossal_lvl, Colossal_kills);
		}
		else {
			RecordOverkillSize_Transient(a_transient, 1.0f, 0.0f);
		}

		const float MaxAllowedSize = 1.0f + (QuestMult + GetSizeFromPerks(a_actor)) * LevelBonus;
		return (MaxAllowedSize + PotionSize + Colossal_kills) * Colossal_lvl;
	}
}

namespace GTS {

	bool ActorMatchesSizeLimitRuleTarget(Actor* a_actor, LSizeLimitRuleTarget_t a_target) {
		return SizeOverride::ActorMatchesRuleTarget(a_actor, a_target);
	}

	void EnsureSizeLimitRulesInitialized() {
		SizeOverride::EnsureInitialized();
	}

	bool SizeLimitRulesActive() {
		return SizeOverride::SizeOverrideEnabled();
	}

	float GetActionCompatibleSizeLimit() {
		return SizeOverride::GetActionFitLimit();
	}

    void UpdateMaxScale(Actor* a_actor, PersistentActorData* a_persistent, TransientActorData* a_transient) {
		GTS_PROFILE_SCOPE("MaxSizeManager: UpdateMaxScale");
		if (!a_actor) {
			return;
		}

		if (!a_persistent) {
			a_persistent = Persistent::GetActorData(a_actor);
		}
		if (!a_persistent) {
			return;
		}
		if (!a_transient) {
			a_transient = Transient::GetActorData(a_actor);
		}

		const bool IsMassBased = Config::Balance.sSizeMode == "kMassBased"; // Should DLL use mass based formula for Player?
		a_persistent->fSizeLimit = ComputeExpectedMaxSize(a_actor, a_persistent, a_transient);
		const float Limit = a_persistent->fSizeLimit;

        const float NaturalScale = get_natural_scale(a_actor, true);
		const float defaultLimit = NaturalScale + ((Limit - 1.0f) * NaturalScale);
		float GetLimit = std::clamp(defaultLimit, 0.1f, DEFAULT_MAX);
		
		if (IsMassBased) {
			GetLimit = get_mass_based_limit(a_persistent, NaturalScale); // Apply Player Mass-Based max size
		}

		float TotalLimit = GetLimit;
		float OverrideLimit = 0.0f;
		bool HasOverrideLimit = false;
		bool LockToNatural = false;

		if (SizeOverride::SizeOverrideEnabled()) {
			const auto resolvedRule = SizeOverride::ResolveRuleForActor(a_actor);
			OverrideLimit = resolvedRule.OverrideLimit;
			LockToNatural = resolvedRule.LockToNatural;

			if (resolvedRule.HasOverrideLimit && OverrideLimit > SizeOverride::SIZE_OVERRIDE_MIN) {
				TotalLimit = OverrideLimit;
				HasOverrideLimit = true;
			}
		}

		Ench_Potions_ApplyBonuses(a_actor, TotalLimit); // Apply after size override, else Butt Crush growth won't be able to surpass size limit for example

		if (std::abs(get_max_scale(a_actor) - TotalLimit) > SIZE_OVERRIDE_RESTORE_EPS) {
			set_max_scale(a_actor, TotalLimit);
		}

		if (LockToNatural && get_target_scale(a_actor) < NaturalScale - SIZE_OVERRIDE_RESTORE_EPS) {
			set_target_scale(a_actor, NaturalScale);
		}

		UpdateSizeOverrideRestore(a_actor, a_transient, OverrideLimit, HasOverrideLimit);
		
    }

	void VisualScale_CheckForSizeAdjustment(Actor* actor, float& ScaleMult) {
		if (SizeOverride::SizeOverrideEnabled()) {
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

