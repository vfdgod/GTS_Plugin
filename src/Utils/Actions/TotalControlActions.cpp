#include "Utils/Actions/TotalControlActions.hpp"

#include "Magic/Effects/Common.hpp"
#include "Managers/Rumble.hpp"

namespace {

	constexpr float Duration = 2.0f;

	float SizeAmplifyBonus(RE::Actor* actor, float targetScale) {
		return GTS::Runtime::HasMagicEffect(actor, GTS::Runtime::MGEF.GTSPotionEffectSizeAmplify) ?
			targetScale * 0.25f + 0.75f : 1.0f;
	}
}

namespace GTS::TotalControlActions {

	void GrowTeammatesOverTime(float a_power) {
		Actor* casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef || a_power <= 0.0f) {
			return;
		}

		for (Actor* targetRef : FindTeammates()) {
			if (!targetRef) {
				continue;
			}

			const float scale = get_visual_scale(targetRef);
			const float volume = std::clamp(scale / 8.0f, 0.20f, 1.0f);
			Runtime::PlaySoundAtNode_FallOff(Runtime::SNDR.GTSSoundGrowth, targetRef, volume, "NPC Pelvis [Pelv]", 0.11f * scale);

			const ActorHandle casterHandle = casterRef->CreateRefHandle();
			const ActorHandle targetHandle = targetRef->CreateRefHandle();
			const std::string name = std::format("GrowFollower_{}", targetRef->formID);

			TaskManager::RunFor(name, Duration, [=](const TaskForUpdate& progressData) {
				if (!casterHandle || !targetHandle) {
					return false;
				}

				Actor* target = targetHandle.get().get();
				Actor* caster = casterHandle.get().get();
				if (!target || !caster) {
					return false;
				}

				const float targetScale = get_target_scale(target);
				const float magicka = std::clamp(GetMagikaPercentage(caster), 0.05f, 1.0f);
				const float bonus = SizeAmplifyBonus(caster, targetScale);
				const float timeDelta = static_cast<float>(progressData.delta * 60.0);

				DamageAV(caster, ActorValue::kMagicka, 0.45f * (targetScale * 0.25f + 0.75f) * magicka * bonus * timeDelta * a_power);
				Grow(target, 0.0030f * magicka * bonus * a_power, 0.0f);
				Rumbling::Once("GrowOtherButton", target, 1.0f, 0.05f);
				return true;
			});
		}
	}

	void ShrinkTeammatesOverTime(float a_power) {
		Actor* casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef || a_power <= 0.0f) {
			return;
		}

		for (Actor* targetRef : FindTeammates()) {
			if (!targetRef) {
				continue;
			}

			const float scale = get_visual_scale(targetRef);
			const float volume = std::clamp(scale * 0.10f, 0.10f, 1.0f);
			Runtime::PlaySoundAtNode_FallOff(Runtime::SNDR.GTSSoundShrink, targetRef, volume, "NPC Pelvis [Pelv]", 0.11f * scale);

			const ActorHandle casterHandle = casterRef->CreateRefHandle();
			const ActorHandle targetHandle = targetRef->CreateRefHandle();
			const std::string name = std::format("ShrinkFollower_{}", targetRef->formID);

			TaskManager::RunFor(name, Duration, [=](const TaskForUpdate&) {
				if (!casterHandle || !targetHandle) {
					return false;
				}

				Actor* target = targetHandle.get().get();
				Actor* caster = casterHandle.get().get();
				if (!target || !caster) {
					return false;
				}

				const float targetScale = get_target_scale(target);
				const float magicka = std::clamp(GetMagikaPercentage(caster), 0.05f, 1.0f);
				const float bonus = SizeAmplifyBonus(caster, targetScale);

				if (targetScale > get_natural_scale(target, true)) {
					DamageAV(caster, ActorValue::kMagicka, 0.25f * (targetScale * 0.25f + 0.75f) * magicka * bonus * TimeScale() * a_power);
					ShrinkActor(target, 0.0030f * magicka * bonus * a_power, 0.0f);
					Rumbling::Once("ShrinkOtherButton", target, 1.0f, 0.05f);
				}
				return true;
			});
		}
	}

	void GrowPlayerOverTime(float a_power) {
		Actor* casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef || a_power <= 0.0f) {
			return;
		}

		const float scale = get_visual_scale(casterRef);
		const float volume = std::clamp(scale * 0.20f, 0.20f, 1.0f);
		Runtime::PlaySoundAtNode_FallOff(Runtime::SNDR.GTSSoundGrowth, casterRef, volume, "NPC Pelvis [Pelv]", 0.11f * scale);

		const ActorHandle casterHandle = casterRef->CreateRefHandle();
		const std::string name = std::format("GrowPlayer_{}", casterRef->formID);

		TaskManager::RunFor(name, Duration, [=](const TaskForUpdate&) {
			if (!casterHandle) {
				return false;
			}

			Actor* caster = casterHandle.get().get();
			if (!caster) {
				return false;
			}

			const float casterScale = get_visual_scale(caster);
			const float stamina = std::clamp(GetStaminaPercentage(caster), 0.05f, 1.0f);
			DamageAV(caster, ActorValue::kStamina, 0.45f * (casterScale * 0.5f + 0.5f) * stamina * TimeScale() * a_power);
			Grow(caster, 0.0030f * stamina * a_power, 0.0f);
			Rumbling::Once("GrowButton", caster, 1.0f, 0.05f);
			return true;
		});
	}

	void ShrinkPlayerOverTime(float a_power, float a_minimumScale) {
		Actor* casterRef = PlayerCharacter::GetSingleton();
		if (!casterRef || a_power <= 0.0f) {
			return;
		}

		const float scale = get_visual_scale(casterRef);
		const float volume = std::clamp(scale * 0.10f, 0.10f, 1.0f);
		Runtime::PlaySoundAtNode_FallOff(Runtime::SNDR.GTSSoundShrink, casterRef, volume, "NPC Pelvis [Pelv]", 0.11f * scale);

		const ActorHandle casterHandle = casterRef->CreateRefHandle();
		const std::string name = std::format("ShrinkPlayer_{}", casterRef->formID);

		TaskManager::RunFor(name, Duration, [=](const TaskForUpdate&) {
			if (!casterHandle) {
				return false;
			}

			Actor* caster = casterHandle.get().get();
			if (!caster) {
				return false;
			}

			const float casterScale = get_visual_scale(caster);
			const float targetScale = get_target_scale(caster);
			const float stamina = std::clamp(GetStaminaPercentage(caster), 0.05f, 1.0f);
			if (targetScale > a_minimumScale) {
				DamageAV(caster, ActorValue::kStamina, 0.25f * (casterScale * 0.5f + 0.5f) * stamina * TimeScale() * a_power);
				ShrinkActor(caster, 0.0020f * stamina * a_power, 0.0f);
				Rumbling::Once("ShrinkButton", caster, 0.60f, 0.05f);
				return true;
			}

			set_target_scale(caster, a_minimumScale);
			return false;
		});
	}
}
