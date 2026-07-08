#include "Managers/AttributeManager.hpp"

#include "Config/Config.hpp"

#include "Magic/Effects/Common.hpp"

#include "Managers/Damage/TinyCalamity.hpp"
#include "Managers/GTSSizeManager.hpp"
#include "Managers/AttackManager.hpp"
#include "Managers/Perks/ShrinkingGaze.hpp"
#include "Utils/Actor/ActorBools.hpp"

using namespace REL;
using namespace GTS;

// TODO move away from polling
namespace {

	float GetMovementSpeedFormula(Actor* actor, float smt_speed, float MovementDebuff) {
		// In short: 1.0 * (size * animation slowdown) * SMT run speed
		float scale = get_giantess_scale(actor);
		float MovementSpeed = soft_core(scale, GetSpeedFromConfig());
		float smt = 1.0f + (smt_speed * 0.33f);

		const float Config = Config::Balance.fSizeSpeedPercentage;
		float speedCompensation = std::lerp(1.0f, scale, Config);

		float power = speedCompensation * 1.0f * (smt * MovementSpeed);
		return power * MovementDebuff;
	}

	float GetMovementSlowdown(Actor* tiny) {
		auto transient = Transient::GetActorData(tiny);
		if (transient) {
			return transient->MovementSlowdown;
		}
		return 1.0f;
	}

	constexpr float kShrinkStealHardDrainThreshold = 0.50f;
	constexpr float kShrinkStealBaseDrainPerTick = 0.05f;
	constexpr float kShrinkStealSeverityDrainPerTick = 0.15f;
	constexpr float kShrinkStealExperienceScale = 0.01f; // 100 resource points = 1.0 GTS XP
	constexpr float kActorValueEpsilon = 1e-3f;

	float GetShrinkStealScaleRatio(Actor* actor) {
		const float naturalScale = std::max(get_natural_scale(actor, true), 0.01f);
		return std::max(get_visual_scale(actor) / naturalScale, 0.0f);
	}

	bool ShouldHardDrainShrinkSteal(float scaleRatio) {
		return scaleRatio <= kShrinkStealHardDrainThreshold + kActorValueEpsilon;
	}

	void FeedPlayerResourceOrExperience(Actor* player, ActorValue av, float drainedValue) {
		if (!player || drainedValue <= kActorValueEpsilon) {
			return;
		}

		const float playerMax = std::max(GetMaxAV(player, av), 0.0f);
		const float playerCurrent = std::clamp(GetAV(player, av), 0.0f, playerMax);
		const float missingValue = std::max(playerMax - playerCurrent, 0.0f);
		const float restoredValue = std::min(drainedValue, missingValue);

		if (restoredValue > kActorValueEpsilon) {
			player->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, restoredValue);
		}

		const float extraValue = drainedValue - restoredValue;
		if (extraValue > kActorValueEpsilon) {
			ModSizeExperience(player, extraValue * kShrinkStealExperienceScale);
		}
	}

	void DrainShrunkActorValue(Actor* actor, Actor* player, ActorValue av, float scaleRatio) {
		const float maxValue = std::max(GetMaxAV(actor, av), 0.0f);
		if (maxValue <= kActorValueEpsilon) {
			return; // Some targets simply do not use this AV, e.g. many animals have no magicka pool.
		}

		const float currentValue = std::clamp(GetAV(actor, av), 0.0f, maxValue);
		if (currentValue <= kActorValueEpsilon) {
			return;
		}

		float drainedValue = currentValue;
		if (!ShouldHardDrainShrinkSteal(scaleRatio)) {
			// Shrunk below natural size: drain continuously.
			// Once it reaches 50% of natural size or less, the entire resource pool is drained.
			const float normalizedSeverity = std::clamp(
				(1.0f - scaleRatio) / (1.0f - kShrinkStealHardDrainThreshold),
				0.0f,
				1.0f
			);
			const float drainStep = maxValue * (kShrinkStealBaseDrainPerTick + normalizedSeverity * kShrinkStealSeverityDrainPerTick) * std::max(TimeScale(), 0.01f);
			drainedValue = std::clamp(drainStep, 0.0f, currentValue);
		}

		if (drainedValue <= kActorValueEpsilon) {
			return;
		}

		DamageAV(actor, av, drainedValue);
		FeedPlayerResourceOrExperience(player, av, drainedValue);
	}

	void ManageShrinkResourceSteal(Actor* actor) {
		if (!Config::Balance.bShrinkStealResources || !actor || actor->IsPlayerRef() || actor->IsDead()) {
			return;
		}

		Actor* player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		const float scaleRatio = GetShrinkStealScaleRatio(actor);
		if (scaleRatio >= 1.0f - kActorValueEpsilon) {
			return;
		}

		DrainShrunkActorValue(actor, player, ActorValue::kStamina, scaleRatio);
		DrainShrunkActorValue(actor, player, ActorValue::kMagicka, scaleRatio);
	}

	void ManageShrinkAttackBlock(Actor* actor) {
		if (!actor || actor->IsPlayerRef()) {
			return;
		}

		auto transient = Transient::GetActorData(actor);
		if (!transient) {
			return;
		}

		const bool shouldBlock = AttackManager::ShouldBlockShrunkAttacks(actor);
		if (shouldBlock) {
			AttackManager::SetAttacksDisabled(actor, true);
			transient->ShrinkAttackBlockActive = true;
			return;
		}

		if (transient->ShrinkAttackBlockActive) {
			AttackManager::SetAttacksDisabled(actor, false);
			transient->ShrinkAttackBlockActive = false;
		}
	}

	void ManagePerkBonuses(Actor* actor) {

		auto& SizeManager = SizeManager::GetSingleton();
		float BalanceModeDiv = SizeManager::BalancedMode() ? 2.0f : 1.0f;
		float gigantism = 1.0f + (Ench_Aspect_GetPower(actor) * 0.30f);

		float BaseGlobalDamage = SizeManager::GetSizeAttribute(actor, SizeAttribute::Normal);
		float BaseSprintDamage = SizeManager::GetSizeAttribute(actor, SizeAttribute::Sprint);
		float BaseFallDamage = SizeManager::GetSizeAttribute(actor, SizeAttribute::JumpFall);

		float ExpectedGlobalDamage = 1.0f;
		float ExpectedSprintDamage = 1.0f;
		float ExpectedFallDamage = 1.0f;

		// -- Normal Damage
		if (Runtime::HasPerkTeam(actor, Runtime::PERK.GTSPerkCruelty)) {
			ExpectedGlobalDamage += 0.15f/BalanceModeDiv;
		}
		if (Runtime::HasPerkTeam(actor, Runtime::PERK.GTSPerkRealCruelty)) {
			ExpectedGlobalDamage += 0.35f/BalanceModeDiv;
		}
		if (IsGrowthSpurtActive(actor)) {
			ExpectedGlobalDamage *= (1.0f + (0.35f/BalanceModeDiv));
		}
		if (Runtime::HasPerkTeam(actor, Runtime::PERK.GTSPerkMightOfGiants)) {
			ExpectedGlobalDamage *= 1.15f; // +15% damage
		}

		// -- Sprint Damage
		if (Runtime::HasPerkTeam(actor, Runtime::PERK.GTSPerkSprintDamageMult1)) {
			ExpectedSprintDamage += 0.25f/BalanceModeDiv;
		}
		// -- Fall Damage
		if (Runtime::HasPerkTeam(actor, Runtime::PERK.GTSPerkCruelFall)) {
			ExpectedFallDamage += 0.3f/BalanceModeDiv;
		}
		// -- Buff by enchantment
		ExpectedGlobalDamage *= gigantism;
		ExpectedSprintDamage *= gigantism;
		ExpectedFallDamage *= gigantism;

		if (BaseGlobalDamage != ExpectedGlobalDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedGlobalDamage, SizeAttribute::Normal);
		}
		if (BaseSprintDamage != ExpectedSprintDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedSprintDamage, SizeAttribute::Sprint);
		}
		if (BaseFallDamage != ExpectedFallDamage) {
			SizeManager.SetSizeAttribute(actor, ExpectedFallDamage, SizeAttribute::JumpFall);
		}
	}

	void UpdateActors(Actor* actor) {
		if (actor) {
			ManagePerkBonuses(actor);
			ManageShrinkResourceSteal(actor);
			ManageShrinkAttackBlock(actor);
			if (actor->IsPlayerRef()) {
				TinyCalamity_BonusSpeed(actor); // Manages SMT bonuses
				if (TinyCalamityActive(actor) && TinyCalamityHasShrinkingGaze(actor)) {
					StartShrinkingGaze(actor);
				}
				if (!TinyCalamitySprintBoostActive(actor)) {
					AttributeManager::OverrideSMTBonus(0.0f);
				}
			}
		}
	}
}


namespace GTS {

	std::string AttributeManager::DebugName() {
		return "::AttributeManager";
	}

	void AttributeManager::Update() {

		static Timer timer = Timer(0.5);

		if (timer.ShouldRunFrame()) { // Run once per 0.5 sec
			for (auto actor: find_actors()) {
				if (actor) {
					if (actor->Is3DLoaded()) {
						UpdateActors(actor);
					}
				}
			}
		}
	}

	void AttributeManager::OverrideSMTBonus(float Value) {
		auto ActorAttributes = Persistent::GetActorData(PlayerCharacter::GetSingleton());
		if (ActorAttributes) {
			ActorAttributes->fSMTRunSpeed = Value;
		}
	}

	float AttributeManager::GetAttributeBonus(Actor* actor, ActorValue av) {

		GTS_PROFILE_SCOPE("AttributeManager: GetAttributeBonus");

		if (!actor) {
			return 1.0f;
		}

		float BalancedMode = SizeManager::BalancedMode() ? 2.0f : 1.0f;
		float natural_scale = get_natural_scale(actor, true);
		float scale = get_giantess_scale(actor);
		if (scale <= 0) {
			scale = 1.0f;
		} 
		if (scale < 1.0f) {
			scale /= natural_scale; 
			// Fix: negative bonuses when natural scale is < 1.0
			// No Fix: 0.91/1.0 = 0.91   (0.91 is just example of current size)
			// Fix:    0.91/0.91(natural size) = 1.0
			// Problem: DR resets after reaching 1.0. Not sure how to fix
		}
		switch (av) {

			case ActorValue::kHealth: {
				float might = 1.0f + Potion_GetMightBonus(actor);

				if (actor->IsPlayerRef()) {
					if (TinyCalamityAttributeBoostActive(actor)) {
						scale += 1.0f;
					}
					if (actor->AsActorState()->IsSprinting() && Runtime::HasPerk(actor, Runtime::PERK.GTSPerkSprintDamageMult1)) {
						scale *= 1.30f;
					}
				}

				scale *= might;
				float resistance = std::clamp(1.0f / scale, 0.001f, 3.0f); // 0.001% as max resistance, -300% is a max vulnerability.
				return resistance;

			}

			case ActorValue::kCarryWeight: {
				const float BonusCarryMult = Config::Balance.fStatBonusCarryWeightMult;
				float power = (BonusCarryMult/BalancedMode);

				float might = 1.0f + Potion_GetMightBonus(actor);

				if (actor->IsPlayerRef() && TinyCalamityAttributeBoostActive(actor)) {
					scale += 3.0f;
				}
				if (scale > 1.0f) {
					return (power*scale*might) + 1.0f - power;
				} else {
					return 1.0f * might; // Don't reduce it if scale is < 1.0
				}
			}

			case ActorValue::kSpeedMult: {
				auto actorData = Persistent::GetActorData(actor);
				float MovementDebuff = GetMovementSlowdown(actor);
				float smt_speed = 0.0f;

				if (actorData) {
					smt_speed = actorData->fSMTRunSpeed;
				}
				float MovementSpeed = GetMovementSpeedFormula(actor, smt_speed, MovementDebuff);

				return MovementSpeed;
			}

			case ActorValue::kAttackDamageMult: {
				if (actor->IsPlayerRef() && TinyCalamityAttributeBoostActive(actor)) {
					scale += 1.0f;
				}
				const float BonusDamageMult = Config::Balance.fStatBonusDamageMult;
				const float DamageStorage = 1.0f + ((BonusDamageMult) * (scale - 1.0f));

				float might = 1.0f + Potion_GetMightBonus(actor);

				if (scale > 1.0f) {
					return DamageStorage * might;
				} else {
					return scale * might;
				}
			}

			case ActorValue::kJumpingBonus: { // Used through MCM only (display bonus jump height)
				float power = 1.0f;
				float defaultjump = 1.0f + (1.0f * (scale - 1) * power);
				float might = 1.0f + Potion_GetMightBonus(actor);
				if (scale > 1.0f) {
					return defaultjump * might;
				} else {
					return scale * might;
				}
			}
			default: {
				return 1.0f;
			}
		}
	}

	float AttributeManager::AlterCarryWeightAV(Actor* actor, ActorValue av, float originalValue) {

		switch (av) {
			case ActorValue::kCarryWeight: {
				const float bonus = AttributeManager::GetAttributeBonus(actor, av);
				auto transient = Transient::GetActorData(actor);
				if (transient != nullptr) {
					transient->CarryWeightBoost = (originalValue * bonus) - originalValue;
				}
				return originalValue * bonus;
			}
			default: return originalValue;

		}

	}

	float AttributeManager::AlterGetBaseAv(Actor* actor, ActorValue av, float originalValue) {

		switch (av) {

			case ActorValue::kHealth: { // 27.03.2024: Health boost is still applied, but for Player only and only if having matching perks

				float perkbonus = GetStolenAttributes_Values(actor, ActorValue::kHealth); // calc health from the perk bonuses
				float finalValue = originalValue + perkbonus; // add flat health on top
				auto transient = Transient::GetActorData(actor);
				if (transient) {
					transient->HealthBoost = finalValue - originalValue;
				}
				return finalValue;
			}
			case ActorValue::kMagicka: {
				float perkbonus = GetStolenAttributes_Values(actor, ActorValue::kMagicka);
				return originalValue + perkbonus;
			}
			case ActorValue::kStamina: {
				float perkbonus = GetStolenAttributes_Values(actor, ActorValue::kStamina);
				return originalValue + perkbonus;
			}

			default: return originalValue;

		}
	}

	float AttributeManager::AlterSetBaseAv(Actor* actor, ActorValue av, float originalValue) {

		switch (av) {

			case ActorValue::kHealth: {
				auto transient = Transient::GetActorData(actor);
				if (transient) {
					float lastEdit = transient->HealthBoost;
					if (originalValue - lastEdit > 0.0f) {
						originalValue -= lastEdit;
						return originalValue;
					}
				}
			}

			default: return originalValue;

		}


	}

	float AttributeManager::AlterMovementSpeed(Actor* actor) {
		if (actor) {
			return AttributeManager::GetAttributeBonus(actor, ActorValue::kSpeedMult);
		}
		return 1.0f;
	}
}
