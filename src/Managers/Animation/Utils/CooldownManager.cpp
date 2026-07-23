#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Constants.hpp"

#include "Config/Config.hpp"

#include "Managers/Animation/AnimationManager.hpp"

using namespace GTS;

namespace {
	float ScaleCooldownWithAnimation(Actor* actor, float cooldown) {
		const float animationSpeed = AnimationManager::GetAnimSpeed(actor);
		return animationSpeed > 0.0f ? cooldown / animationSpeed : cooldown;
	}

	float Calculate_FootstepTimer(Actor* actor) {
		return ScaleCooldownWithAnimation(actor, 0.2f);
	}
	float Calculate_EmotionCooldown(Actor* actor) {
		return ScaleCooldownWithAnimation(actor, EMOTION_COOLDOWN);
	}
	float Calculate_EmotionCooldown_Long(Actor* actor) {
		return ScaleCooldownWithAnimation(actor, EMOTION_COOLDOWN_LONG);
	}
	float Calculate_LaughCooldown(Actor* actor) {
		return ScaleCooldownWithAnimation(actor, LAUGH_COOLDOWN);
	}
	float Calculate_MoanCooldown(Actor* actor) {
		return ScaleCooldownWithAnimation(actor, MOAN_COOLDOWN);
    }

	bool IsCooldownDisabledByCheat(CooldownSource source) {
        if (Enum_Contains<CooldownSource>(source, "Action")) {
            return true;
        }

        switch (source) {
            case CooldownSource::Misc_ShrinkOutburst:
            case CooldownSource::Misc_ShrinkOutburst_Forced:
            case CooldownSource::Misc_TinyCalamity_WrathfulCalamity:
            case CooldownSource::Misc_TinyCalamity_Shrink:
                return true;
            default:
                return false;
		}
	}

	float Calculate_BreastSuffocateCooldown(Actor* actor) {
		return Calculate_BreastActionCooldown(actor, 0);
	}

	float Calculate_BreastVoreCooldown(Actor* actor) {
		return Calculate_BreastActionCooldown(actor, 1);
	}

	float Calculate_BreastAbsorbCooldown(Actor* actor) {
		return Calculate_BreastActionCooldown(actor, 2);
	}

	struct CooldownDescriptor {
		double CooldownData::* timestamp;
		float fixedDuration = 0.0f;
		float (*calculateDuration)(Actor*) = nullptr;

		float Duration(Actor* actor) const {
			return calculateDuration ? calculateDuration(actor) : fixedDuration;
		}
	};

	std::optional<CooldownDescriptor> GetCooldownDescriptor(CooldownSource source) {
		using Data = CooldownData;
		switch (source) {
			case CooldownSource::Damage_Launch: return CooldownDescriptor{ &Data::lastLaunchTime, LAUNCH_COOLDOWN };
			case CooldownSource::Damage_Hand: return CooldownDescriptor{ &Data::lastHandDamageTime, HANDDAMAGE_COOLDOWN };
			case CooldownSource::Damage_Thigh: return CooldownDescriptor{ &Data::lastThighDamageTime, THIGHDAMAGE_COOLDOWN };
			case CooldownSource::Push_Basic: return CooldownDescriptor{ &Data::lastPushTime, PUSH_COOLDOWN };
			case CooldownSource::Action_ButtCrush: return CooldownDescriptor{ &Data::lastButtCrushTime, 0.0f, Calculate_ButtCrushTimer };
			case CooldownSource::Action_HealthGate: return CooldownDescriptor{ &Data::lastHealthGateTime, HEALTHGATE_COOLDOWN };
			case CooldownSource::Action_ScareOther: return CooldownDescriptor{ &Data::lastScareTime, SCARE_COOLDOWN };
			case CooldownSource::Action_HugAbsorbOther: return CooldownDescriptor{ &Data::lastAbsorbTime, 0.0f, Calculate_HugCrushCooldown };
			case CooldownSource::Action_Breasts_Absorb: return CooldownDescriptor{ &Data::lastBreastAbsorbTime, 0.0f, Calculate_BreastAbsorbCooldown };
			case CooldownSource::Action_Breasts_Suffocate: return CooldownDescriptor{ &Data::lastBreastSuffocateTime, 0.0f, Calculate_BreastSuffocateCooldown };
			case CooldownSource::Action_Breasts_Vore: return CooldownDescriptor{ &Data::lastBreastVoreTime, 0.0f, Calculate_BreastVoreCooldown };
			case CooldownSource::Action_Hugs: return CooldownDescriptor{ &Data::lastHugTime, HUGS_COOLDOWN };
			case CooldownSource::Emotion_Laugh: return CooldownDescriptor{ &Data::lastLaughTime, 0.0f, Calculate_LaughCooldown };
			case CooldownSource::Emotion_Moan: return CooldownDescriptor{ &Data::lastMoanTime, 0.0f, Calculate_MoanCooldown };
			case CooldownSource::Emotion_Moan_Crush: return CooldownDescriptor{ &Data::lastMoanCrushTime, MOAN_CRUSH_COOLDOWN };
			case CooldownSource::Misc_RevertSound: return CooldownDescriptor{ &Data::lastRevertTime, SOUND_COOLDOWN };
			case CooldownSource::Misc_GrowthSound: return CooldownDescriptor{ &Data::lastSoundGrowthTime, GROW_SOUND_COOLDOWN };
			case CooldownSource::Misc_BeingHit: return CooldownDescriptor{ &Data::lastHitTime, HIT_COOLDOWN };
			case CooldownSource::Misc_AiGrowth: return CooldownDescriptor{ &Data::lastGrowthTime, AI_GROWTH_COOLDOWN };
			case CooldownSource::Misc_ShrinkOutburst: return CooldownDescriptor{ &Data::lastOutburstTime, 0.0f, Calculate_ShrinkOutburstTimer };
			case CooldownSource::Misc_ShrinkOutburst_Forced: return CooldownDescriptor{ &Data::lastForceOutburstTime, SHRINK_OUTBURST_COOLDOWN_FORCED };
			case CooldownSource::Misc_ShrinkParticle: return CooldownDescriptor{ &Data::lastShrinkParticleTime, SHRINK_PARTICLE_COOLDOWN };
			case CooldownSource::Misc_ShrinkParticle_Animation: return CooldownDescriptor{ &Data::lastAnimShrinkParticleTime, SHRINK_PARTICLE_COOLDOWN_ANIM };
			case CooldownSource::Misc_ShrinkParticle_Gaze: return CooldownDescriptor{ &Data::lastGazeShrinkParticleTime, SHRINK_PARTICLE_COOLDOWN_GAZE };
			case CooldownSource::Misc_TinyCalamity_WrathfulCalamity: return CooldownDescriptor{ &Data::lastTinyCalamityOneShotTime, TINYCALAMITY_ONESHOT_COOLDOWN };
			case CooldownSource::Misc_TinyCalamity_Shrink: return CooldownDescriptor{ &Data::lastTinyCalamityShrinkTime, TINYCALAMITY_SHRINK_COOLDOWN };
			case CooldownSource::Footstep_Right: return CooldownDescriptor{ &Data::lastFootstepTime_R, 0.0f, Calculate_FootstepTimer };
			case CooldownSource::Footstep_Left: return CooldownDescriptor{ &Data::lastFootstepTime_L, 0.0f, Calculate_FootstepTimer };
			case CooldownSource::Footstep_JumpLand: return CooldownDescriptor{ &Data::lastJumplandTime, 0.0f, Calculate_FootstepTimer };
			case CooldownSource::Emotion_Voice: return CooldownDescriptor{ &Data::lastEmotionTime, 0.0f, Calculate_EmotionCooldown };
			case CooldownSource::Emotion_Voice_Long: return CooldownDescriptor{ &Data::lastEmotionTime_Long, 0.0f, Calculate_EmotionCooldown_Long };
		}
		return std::nullopt;
	}

	std::optional<double> GetCooldownEndTime(Actor* actor, CooldownSource source) {
		if (!actor) {
			return std::nullopt;
		}
		const auto descriptor = GetCooldownDescriptor(source);
		if (!descriptor) {
			return std::nullopt;
		}

		auto& data = CooldownManager::GetSingleton().GetCooldownData(actor);
		return data.*descriptor->timestamp + descriptor->Duration(actor);
	}
}

namespace GTS {

	std::string CooldownManager::DebugName() {
		return "::CooldownManager";
	}

	CooldownData& CooldownManager::GetCooldownData(Actor* actor) {
		return this->m_cooldownData.try_emplace(actor).first->second;
	}

	void CooldownManager::Reset() {
		this->m_cooldownData.clear();
        logger::info("Cooldowns cleared");
    }
    float Calculate_ShrinkOutburstTimer(Actor* actor) {
        bool DarkArts3 = Runtime::HasPerk(actor, Runtime::PERK.GTSPerkDarkArtsAug3);
        bool HealthRegen = Runtime::HasPerk(actor, Runtime::PERK.GTSPerkGrowthAug2);
        bool DarkArts_Legendary = Runtime::HasPerk(actor, Runtime::PERK.GTSPerkDarkArtsLegendary);
        float reduction = 1.0f;
        if (DarkArts3) {
            reduction = 0.7f;
        }
        if (HealthRegen && IsGrowthSpurtActive(actor)) {
            reduction *= 0.75f;
        }
        if (DarkArts_Legendary) {
            reduction *= 0.75f;
        }
        return SHRINK_OUTBURST_COOLDOWN * reduction;
    }

    float Calculate_BreastActionCooldown(Actor* giant, int type) {
        float Cooldown = 1.0;
        float mastery = 0.0;
        switch (type) {
            case 0:
                Cooldown = BREAST_SUFFOCATE_OTHER_COOLDOWN;
            break;
            case 1:
                Cooldown = BREAST_VORE_OTHER_COOLDOWN;
            break;
            case 2:
                Cooldown = BREAST_ABSORB_OTHER_COOLDOWN;
            break;
            default: 
                Cooldown = BREAST_SUFFOCATE_OTHER_COOLDOWN;
            break;   
        }

        if (Runtime::HasPerk(giant, Runtime::PERK.GTSPerkBreastsMastery1)) {
            float level = GetGtsSkillLevel(giant) - 40.0f; // Start past level 40
            mastery = std::clamp(level * 0.01f, 0.0f, 0.6f);
        }
        float reduction = 1.0f - mastery;

        return Cooldown * reduction;
    }

    float Calculate_HugCrushCooldown(Actor* giant) {
        float mastery = std::clamp(GetGtsSkillLevel(giant) * 0.01f, 0.0f, 1.0f) * 0.666f;
        float reduction = 1.0f - mastery; // Up to 15 seconds at level 100

        return ABSORB_OTHER_COOLDOWN * reduction;
    }

    float Calculate_ButtCrushTimer(Actor* actor) {
		bool lvl70 = Runtime::HasPerk(actor, Runtime::PERK.GTSPerkButtCrushAug3);
		bool lvl100 = Runtime::HasPerk(actor, Runtime::PERK.GTSPerkButtCrushAug4);
		float reduction = 1.0f;
		if (lvl100) { // 15% reduction
			reduction -= 0.15f;
		} if (lvl70) { // 10% reduction
			reduction -= 0.10f;
		} 
		return BUTTCRUSH_COOLDOWN * reduction;
	}

	void CooldownManager::ResetActor(Actor* actor) {
		if (actor) {
			this->m_cooldownData.erase(actor);
        }
    }

	void ApplyActionCooldown(Actor* giant, CooldownSource source) {
		if (!giant || (!Config::Advanced.bCooldowns && IsCooldownDisabledByCheat(source))) {
			return;
		}

		if (const auto descriptor = GetCooldownDescriptor(source)) {
			auto& data = CooldownManager::GetSingleton().GetCooldownData(giant);
			data.*descriptor->timestamp = Time::WorldTimeElapsed();
		}
	}

    double GetRemainingCooldown(Actor* giant, CooldownSource source) {
        if (!Config::Advanced.bCooldowns && IsCooldownDisabledByCheat(source)) {
            return 0.0;
        }

		const auto endTime = GetCooldownEndTime(giant, source);
		return endTime ? *endTime - Time::WorldTimeElapsed() : 0.0;
	}

    bool IsActionOnCooldown(Actor* giant, CooldownSource source) {

        // Debug cheat: disable gameplay cooldowns while keeping internal anti-spam timers intact.
        if (!Config::Advanced.bCooldowns && IsCooldownDisabledByCheat(source)) {
            return false;
        }

		const auto endTime = GetCooldownEndTime(giant, source);
		return endTime && Time::WorldTimeElapsed() <= *endTime;
	}
}
