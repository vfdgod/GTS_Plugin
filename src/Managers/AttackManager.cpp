#include "Managers/AttackManager.hpp"

#include "Config/Config.hpp"
#include "Managers/MaxSizeManager.hpp"

using namespace GTS;

namespace {
	constexpr float kShrinkAttackBlockEpsilon = 1e-3f;

	bool IsShrunkBelowNaturalScale(Actor* a_Actor) {
		const float naturalScale = std::max(get_natural_scale(a_Actor, true), 0.01f);
		return get_visual_scale(a_Actor) < naturalScale - kShrinkAttackBlockEpsilon;
	}

	void SetAttackFlags(Actor* a_Actor, bool a_Disabled) {
		if (!a_Actor) {
			return;
		}

		auto& flags = a_Actor->GetActorRuntimeData().boolFlags;
		if (a_Disabled) {
			flags.set(Actor::BOOL_FLAGS::kAttackingDisabled);
			flags.set(Actor::BOOL_FLAGS::kCastingDisabled);
		}
		else {
			flags.reset(Actor::BOOL_FLAGS::kAttackingDisabled);
			flags.reset(Actor::BOOL_FLAGS::kCastingDisabled);
		}
	}

	void DisableAttacks_Melee(Actor* a_Giant, float a_SizeDiff, float a_Threshold, bool a_Reset) {

		if (a_Reset) {
			a_Giant->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kAttackingDisabled);
			return;
		}

		const float Random = RandomFloat(0.0f, 20.0f);
		if (a_SizeDiff >= 2.5f && Random <= a_Threshold) {
			a_Giant->GetActorRuntimeData().boolFlags.set(Actor::BOOL_FLAGS::kAttackingDisabled);
		}
		else {
			a_Giant->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kAttackingDisabled);
		}
	}
	void DisableAttacks_Magic(Actor* a_Giant, float a_SizeDiff, float a_Threshold, bool a_Reset) {

		if (a_Reset) {
			a_Giant->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kCastingDisabled);
			return;
		}

		const float Random = RandomFloat(0.0f, 20.0f);
		if (a_SizeDiff >= 2.5f && Random <= a_Threshold) {
			a_Giant->GetActorRuntimeData().boolFlags.set(Actor::BOOL_FLAGS::kCastingDisabled);
		}
		else {
			a_Giant->GetActorRuntimeData().boolFlags.reset(Actor::BOOL_FLAGS::kCastingDisabled);
		}
	}
}

namespace GTS {

	bool AttackManager::ShouldBlockShrunkAttacks(Actor* a_Actor) {
		return Config::Balance.bShrinkDisableAttacks &&
			a_Actor &&
			!a_Actor->IsPlayerRef() &&
			!a_Actor->IsDead() &&
			IsShrunkBelowNaturalScale(a_Actor);
	}

	bool AttackManager::ShouldForceFollowerGTActions(Actor* a_Actor) {
		if (!Config::AI.bFollowersGTOnly ||
			!SizeLimitRulesActive() ||
			!Config::AI.bEnableActionAI ||
			!a_Actor ||
			a_Actor->IsPlayerRef() ||
			a_Actor->IsDead() ||
			!IsTeammate(a_Actor) ||
			!IsHumanoid(a_Actor) ||
			get_visual_scale(a_Actor) < 1.0f) {
			return false;
		}

		return a_Actor->IsInCombat() || a_Actor->GetActorRuntimeData().currentCombatTarget.get().get() != nullptr;
	}

	void AttackManager::SetAttacksDisabled(Actor* a_Actor, bool a_Disabled) {
		SetAttackFlags(
			a_Actor,
			a_Disabled ||
				AttackManager::ShouldBlockShrunkAttacks(a_Actor) ||
				AttackManager::ShouldForceFollowerGTActions(a_Actor)
		);
	}

	void AttackManager::UpdateFollowerCombatAIRestrictions(Actor* a_Actor) {
		if (!a_Actor || a_Actor->IsPlayerRef()) {
			return;
		}

		auto transient = Transient::GetActorData(a_Actor);
		if (!transient) {
			return;
		}

		const bool shouldRestrict = ShouldForceFollowerGTActions(a_Actor);
		constexpr auto noAIAcquireFlag = static_cast<std::uint32_t>(Actor::RecordFlags::kNoAIAcquire);

		if (shouldRestrict) {
			SetAttackFlags(a_Actor, true);
			if ((a_Actor->formFlags & noAIAcquireFlag) == 0) {
				a_Actor->formFlags |= noAIAcquireFlag;
				transient->FollowerNoAIAcquireAdded = true;
			}

			if (auto state = a_Actor->AsActorState(); state && state->actorState2.weaponState != WEAPON_STATE::kSheathed) {
				state->actorState2.weaponState = WEAPON_STATE::kWantToSheathe;
			}
			transient->FollowerGTOnlyActive = true;
			return;
		}

		if (transient->FollowerNoAIAcquireAdded) {
			a_Actor->formFlags &= ~noAIAcquireFlag;
			transient->FollowerNoAIAcquireAdded = false;
		}
		if (transient->FollowerGTOnlyActive) {
			SetAttacksDisabled(a_Actor, false);
			transient->FollowerGTOnlyActive = false;
		}
	}

	void AttackManager::PreventAttacks(Actor* a_Giant, Actor* a_Tiny) {

		if (ShouldBlockShrunkAttacks(a_Giant)) {
			SetAttackFlags(a_Giant, true);
			return;
		}

		if (a_Giant && !a_Giant->IsPlayerRef() && IsHumanoid(a_Giant)) {
			// This mode owns both flags completely; the two legacy attack-blocking options are ignored.
			if (Config::AI.bFollowersGTOnly) {
				SetAttacksDisabled(a_Giant, false);
				return;
			}

			//If disabled in settings each call to this should always enable instead.
			if (!Config::AI.bDisableAttacks) {
				DisableAttacks_Melee(a_Giant, 0.0f, 0.0f, true);
				DisableAttacks_Magic(a_Giant, 0.0f, 0.0f, true);
				return;
			}

			if (Config::AI.bAlwaysDisableAttacks) { // If this option is on, always prevent attacks past 2.5x scale
				const float VisualScale = get_visual_scale(a_Giant);
				constexpr float Threshold = 2.5f;
				if (VisualScale >= Threshold) {
					// past threshold, disable all attacks
					a_Giant->GetActorRuntimeData().boolFlags.set(Actor::BOOL_FLAGS::kAttackingDisabled);
					a_Giant->GetActorRuntimeData().boolFlags.set(Actor::BOOL_FLAGS::kCastingDisabled);
				} else {
					// let RNG decide
					DisableAttacks_Melee(a_Giant, VisualScale, Threshold, false);
					DisableAttacks_Magic(a_Giant, VisualScale, Threshold, false);
				}
				return;
			}

			if (a_Tiny) {

				const float SizeDiff = get_scale_difference(a_Giant, a_Tiny, SizeType::VisualScale, true, false);
				const float Threshold = 2.5f * (SizeDiff - 2.5f);
				DisableAttacks_Melee(a_Giant, SizeDiff, Threshold, false);
				DisableAttacks_Magic(a_Giant, SizeDiff, Threshold, false);

			}
			// If Tiny is nullptr, we count it as 'enable attacks back'
			else {

				DisableAttacks_Melee(a_Giant, 0.0f, 0.0f, true);
				DisableAttacks_Magic(a_Giant, 0.0f, 0.0f, true);

			}
		}
	}
}
