#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Managers/Damage/CollisionDamage.hpp"

#include "Hooks/Experiments/Experiments_FootColliders.hpp"

#include "Config/Config.hpp"

#include "Managers/Damage/SizeHitEffects.hpp"
#include "Managers/Damage/TinyCalamity.hpp"
#include "Managers/Audio/GoreAudio.hpp"
#include "Managers/CrushManager.hpp"
#include "Managers/GTSSizeManager.hpp"

#include "Magic/Effects/Common.hpp"
#include "Scale/Scale.hpp"
#include "Managers/Contact/ColliderUtils.hpp"

#include "Utils/DeathReport.hpp"
#include "Utils/MovementForce.hpp"
#include "Utils/Actor/GTSUtils.hpp"


using namespace GTS;

namespace {
	bool IsIdleDamage(DamageSource cause) {
		return cause == DamageSource::FootIdleL || cause == DamageSource::FootIdleR;
	}
	bool StrongGore(DamageSource cause) {
		bool Strong = false;
		switch (cause) {
			case DamageSource::FootGrindedRight_Impact:
			case DamageSource::FootGrindedLeft_Impact:
			case DamageSource::RightFinger_Impact:
			case DamageSource::LeftFinger_Impact:
			case DamageSource::HandCrawlRight:
			case DamageSource::HandCrawlLeft:
			case DamageSource::KneeDropRight:
			case DamageSource::KneeDropLeft:
			case DamageSource::KneeRight:
			case DamageSource::KneeLeft:
			case DamageSource::HandDropRight:
			case DamageSource::HandDropLeft:
			case DamageSource::HandSlamRight:
			case DamageSource::HandSlamLeft:
			case DamageSource::CrushedRight:
			case DamageSource::CrushedLeft:
			case DamageSource::WalkRight:
			case DamageSource::WalkLeft:
			case DamageSource::BodyCrush:
			case DamageSource::BreastImpact:
			case DamageSource::Booty:
				Strong = true;
			break;
		}
		return Strong;
	}
	bool Allow_Damage(Actor* giant, Actor* tiny, DamageSource cause, float difference) {
		float threshold = 3.0f;

		if (DisallowSizeDamage(giant, tiny)) {
			return false;
		}

		if (difference > threshold) {
			return true;
		}

		bool PreventDamage = false;

		switch (cause) {
			case DamageSource::WalkLeft:
			case DamageSource::WalkRight:
				PreventDamage = true;
			break;
			case DamageSource::KneeLeft:
			case DamageSource::KneeRight:
				PreventDamage = true;
			break;
			case DamageSource::HandCrawlLeft:
			case DamageSource::HandCrawlRight:
				PreventDamage = true;
			break;
		}
		if (PreventDamage) {
			// goal of this function is to deal heavily decreased damage on normal walk footsteps to actors
			// so it won't look silly by dealing 30 damage by briefly colliding with others
			if (difference > 1.4f) {
				InflictSizeDamage(giant, tiny, difference * 0.35f);
			} 
			return false;
		}
		return true;
	}

	float GetPostTinyCalamityShrinkDifference(Actor* tiny, float original_difference) {
		const float visual_tiny_scale = get_visual_scale(tiny) * GetSizeFromBoundingBox(tiny);
		const float target_tiny_scale = get_target_scale(tiny) * GetSizeFromBoundingBox(tiny);
		if (visual_tiny_scale <= 0.0f || target_tiny_scale <= 0.0f || target_tiny_scale >= visual_tiny_scale) {
			return original_difference;
		}

		// Tiny Calamity shrink updates target scale first; use that for the same stomp's crush check.
		return original_difference * (visual_tiny_scale / target_tiny_scale);
	}

	float GetFootActionDamageLimitPercent(FootActionDamageLimitKind damage_limit_kind) {
		if (!Config::Advanced.bFootActionDamageLimit) {
			return 0.0f;
		}

		bool enabled = false;
		switch (damage_limit_kind) {
			case FootActionDamageLimitKind::NormalStomp:
				enabled = Config::Advanced.bFootActionDamageLimitNormal;
				break;
			case FootActionDamageLimitKind::StrongStomp:
				enabled = Config::Advanced.bFootActionDamageLimitStrong;
				break;
			case FootActionDamageLimitKind::Trample:
				enabled = Config::Advanced.bFootActionDamageLimitTrample;
				break;
			default:
				break;
		}

		if (!enabled) {
			return 0.0f;
		}
		return std::clamp(Config::Advanced.fFootActionDamageLimitMaxHealthPercent, 1.0f, 100.0f);
	}

	bool ApplyHighHeelBonus(Actor* giant, DamageSource cause) {
		bool HighHeel = false;
		switch (cause) {
			case DamageSource::CrushedRight:
				HighHeel = true;
			break;
			case DamageSource::CrushedLeft:
				HighHeel = true;
			break;
			case DamageSource::WalkRight:
				HighHeel = true;
			break;
			case DamageSource::WalkLeft:
				HighHeel = true;
			break;
		}
		return HighHeel;
	}

	void ModVulnerability(Actor* giant, Actor* tiny, float damage) {
		if (Runtime::HasPerkTeam(giant, Runtime::PERK.GTSPerkGrowingPressure)) {
			auto& sizemanager = SizeManager::GetSingleton();

			if (Runtime::HasPerkTeam(giant, Runtime::PERK.GTSPerkRavagingInjuries) && giant->AsActorState()->IsSprinting() && !AnimationVars::General::IsGTSBusy(giant)) {
				damage *= 3.0f; // x3 stronger during sprint
			}

			sizemanager.ModSizeVulnerability(tiny, damage * 0.0010f);
		}
	}

	float HighHeels_PerkDamage(Actor* giant, DamageSource Cause) {
		float value = 1.0f;
		bool perk = Runtime::HasPerkTeam(giant, Runtime::PERK.GTSPerkHighHeels);
		bool rumbling_feet = Runtime::HasPerkTeam(giant, Runtime::PERK.GTSPerkRumblingFeet);
		bool matches = false;

		switch (Cause) {
			case DamageSource::CrushedRight:
				matches = true;
			break;
			case DamageSource::CrushedLeft:
				matches = true;
			break;
			case DamageSource::WalkRight:
				matches = true;
			break;
			case DamageSource::WalkLeft:
				matches = true;
			break;
		}
		if (matches) {
			if (rumbling_feet) {
				value += 0.25f; // 25% bonus damage if we have lvl 65 perk
			}
			if (perk) {
				value += 0.15f; // 15% bonus damage if we have High Heels perk
			}
		}
		
		return value;
	}

	bool IsFootIdleDamageSource(DamageSource cause) {
		return cause == DamageSource::FootIdleL || cause == DamageSource::FootIdleR;
	}

	bool ShouldIgnoreNaturalScaleFootIdle(Actor* giant, Actor* tiny, DamageSource cause) {
		if (!TinyCalamityIgnoreNaturalFootIdle(giant)) {
			return false;
		}
		if (!TinyCalamityActionBoostActive(giant) || !IsFootIdleDamageSource(cause)) {
			return false;
		}

		constexpr float NATURAL_SCALE_EPS = 0.02f;
		const float naturalScale = get_natural_scale(tiny, true);
		const float visualScale = get_visual_scale(tiny);
		return visualScale >= naturalScale * (1.0f - NATURAL_SCALE_EPS);
	}
}


namespace GTS {
	float CollisionDamage::ToHavok() {
		const float toHavok = 1.0f / *reinterpret_cast<const float*>(RE::Offset::Havok::WorldScaleInverse.address());
		return toHavok;
	}
	void CollisionDamage::DebugCollision(RE::bhkWorld* world, Actor* actor, std::vector<NiPoint3> CoordsToCheck, float maxFootDistance, float toHavok, bool condition) {
		if (condition) {
			constexpr int duration = 300;
				for (const auto& footPoint : CoordsToCheck) {
					DebugDraw::DrawSphere(glm::vec3(footPoint.x, footPoint.y, footPoint.z), maxFootDistance, duration);
				}
				for (auto& otherActor : find_actors()) {
					if (otherActor == actor) continue;
					if (auto shapeData = GetControllerShapeData(otherActor, toHavok)) {
						BSWriteLockGuard lock(world->worldLock);
						DebugDrawShape(shapeData->shape, shapeData->transform, shapeData->angleZ, 1.0f / toHavok, duration);
					}
				}
			}
		}
	
	bool CollisionDamage::HasCollided(Actor* actor, Actor* otherActor, RE::bhkWorld* world, std::vector<NiPoint3> CoordsToCheck, NiPoint3 giantLocation, float giantScale, float SCALE_RATIO, float maxFootDistance, float maxCheckDistanceSq, float sphereRadiusSq, float toHavok) {
		// Broad-phase distance cull
		NiPoint3 diff = otherActor->GetPosition() - giantLocation;
		float distanceSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
		if (distanceSq > maxCheckDistanceSq) return false;

		// Scale ratio cull
		float tinyScale = get_visual_scale(otherActor) * GetSizeFromBoundingBox(otherActor);
		if (giantScale / tinyScale <= SCALE_RATIO) return false;
		if (auto shapeData = GetControllerShapeData(otherActor, toHavok)) {
			BSWriteLockGuard lock(world->worldLock);
			for (const NiPoint3& footPt : CoordsToCheck) {
				hkVector4 center;
				center.quad = _mm_mul_ps(_mm_set_ps(0.0f, footPt.z, footPt.y, footPt.x), _mm_set1_ps(toHavok));
				if (SphereOverlapsShape(shapeData->shape, center, sphereRadiusSq, shapeData->transform, shapeData->angleZ)) {
					return true;
					break;
				}
			}
		} else {
			// Fallback for ragdolled/dead actors with no controller
			NiPoint3 actorLocation = otherActor->GetPosition();
			for (const NiPoint3& footPt : CoordsToCheck) {
				if ((footPt - actorLocation).Length() - Collision_Distance_Override <= maxFootDistance) {
					return true;
					break;
				}
			}
		}
		return false;
	}
	// Safer optimization that preserves original behavior
	void CollisionDamage::DoFootCollision(Actor* actor, float damage, float radius, int random, float bbmult, float crush_threshold, DamageSource Cause, bool Right, bool ApplyCooldown, bool ignore_rotation, bool SupportCalamity, FootActionDamageLimitKind damage_limit_kind) {

		if (!actor) return;
		
		float giantScale = get_visual_scale(actor) * GetSizeFromBoundingBox(actor);
		constexpr float BASE_CHECK_DISTANCE = 180.0f;
		const float toHavok = ToHavok();
		float SCALE_RATIO = 1.15f;
		float Calamity = 1.0f;

		bool SMT = TinyCalamityActionBoostActive(actor) || TinyCalamityActive(actor);
		if (SMT) {
			if (SupportCalamity) Calamity = 4.0f;
			giantScale += 0.20f;
			SCALE_RATIO = 0.7f;
		}

		float maxFootDistance = radius * giantScale;
		std::vector<NiPoint3> CoordsToCheck = GetFootCoordinates(actor, Right, ignore_rotation);
		if (CoordsToCheck.empty()) return;

		auto* world = actor->GetParentCell() ? actor->GetParentCell()->GetbhkWorld() : nullptr;
		if (!world) return;

		if (!IsIdleDamage(Cause)) {
			const bool Condition = DebugDraw::CanDraw(actor, DebugDraw::DrawTarget::kAnyGTS);
			DebugCollision(world, actor, CoordsToCheck, maxFootDistance, toHavok, Condition);
		}

		NiPoint3 giantLocation = actor->GetPosition();
		float maxCheckDistance = BASE_CHECK_DISTANCE * giantScale;
		float maxCheckDistanceSq = maxCheckDistance * maxCheckDistance;
		
		const float sphereRadiusHk = maxFootDistance * toHavok;
		const float sphereRadiusSq = sphereRadiusHk * sphereRadiusHk;

		for (auto& otherActor : find_actors()) {
			if (otherActor == actor) continue;

			bool HitDetected = HasCollided(actor, otherActor, world, CoordsToCheck, giantLocation, giantScale, SCALE_RATIO, maxFootDistance, maxCheckDistanceSq, sphereRadiusSq, toHavok);

			// Skip natural-scale foot idle targets when configured.
			if (ShouldIgnoreNaturalScaleFootIdle(actor, otherActor, Cause)) continue;

			// Calamity seek runs regardless of hit result
			if (SupportCalamity && SMT) {
				TinyCalamity_SeekForShrink(actor, otherActor, damage, maxFootDistance * Calamity, Cause, Right, ApplyCooldown, ignore_rotation);
			}

			if (HitDetected) {
				if (ApplyCooldown) {
					if (!IsActionOnCooldown(otherActor, CooldownSource::Damage_Thigh)) {
						Utils_PushCheck(actor, otherActor, Get_Bone_Movement_Speed(actor, Cause));
						DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, true, damage_limit_kind);
						ApplyActionCooldown(otherActor, CooldownSource::Damage_Thigh);
					}
				}
				else {
					Utils_PushCheck(actor, otherActor, Get_Bone_Movement_Speed(actor, Cause));
					DoSizeDamage(actor, otherActor, damage, bbmult, crush_threshold, random, Cause, true, damage_limit_kind);
				}
			}
		}
	}

	void CollisionDamage::DoSizeDamage(Actor* giant, Actor* tiny, float damage, float bbmult, float crush_threshold, int random, DamageSource Cause, bool apply_damage, FootActionDamageLimitKind damage_limit_kind) { // Applies damage and crushing
		GTS_PROFILE_SCOPE("CollisionDamage: DoSizeDamage");
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (giant == tiny) {
			return;
		}
		if (!tiny->Is3DLoaded() || !giant->Is3DLoaded()) {
			return;
		}
		if (!CanDoDamage(giant, tiny, true) || IsBetweenBreasts(tiny)) { // disallow 
			return;
		}
		
		bool SMT = TinyCalamityActionBoostActive(giant);
		auto& sizemanager = SizeManager::GetSingleton();
		float size_difference = get_scale_difference(giant, tiny, SizeType::VisualScale, false, true);

		float size_threshold = 1.25f;

		if (SMT) {
			size_threshold = 0.9f;
		}

		if (size_difference > size_threshold) {
			if (Allow_Damage(giant, tiny, Cause, size_difference)) {
				float damagebonus = HighHeels_PerkDamage(giant, Cause); // 15% bonus HH damage if we have perk

				float vulnerability = 1.0f + sizemanager.GetSizeVulnerability(tiny); // Get size damage debuff from enemy
				float normaldamage = std::clamp(SizeManager::GetSizeAttribute(giant, SizeAttribute::Normal) * 0.30f, 0.30f, 1000000.0f);

				float highheelsdamage = 1.0f;
				if (ApplyHighHeelBonus(giant, Cause)) {
					highheelsdamage = GetHighHeelsBonusDamage(giant, true);
				}

				float sprintdamage = 1.0f; // default Sprint damage of 1.0
				float weightdamage = 1.0f + (giant->GetWeight()*0.01f);

				if (giant->AsActorState()->IsSprinting()) {
					sprintdamage = 1.5f * SizeManager::GetSizeAttribute(giant, SizeAttribute::Sprint);
					damage *= 1.5f;
				}

				float Might = 1.0f + Potion_GetMightBonus(giant);
				float damage_result = (damage * size_difference * damagebonus) * (normaldamage * sprintdamage) * (highheelsdamage * weightdamage) * vulnerability;

				damage_result *= Might;
				damage_result = LimitSizeDamageToMaxHealthPercent(giant, tiny, damage_result, GetFootActionDamageLimitPercent(damage_limit_kind));

				TinyCalamity_ShrinkActor(giant, tiny, damage_result * 0.35f * Config::Balance.fSizeDamageMult);

				if (giant->IsSneaking()) {
					damage_result *= 0.85f;
				}

				// ^ Chance to break bonues and inflict additional damage, as well as making target more vulerable to size damage

				if (!tiny->IsDead()) {
					float experience = std::clamp(damage_result/500, 0.0f, 0.05f);
					ModSizeExperience(giant, experience);
				}

				if (tiny->IsPlayerRef() && GetAV(tiny, ActorValue::kStamina) > 2.0f) {
					DamageAV(tiny, ActorValue::kStamina, damage_result * 2.0f);
					damage_result -= GetAV(tiny, ActorValue::kStamina); // Reduce damage by stamina amount

					damage_result = std::max<float>(damage_result, 0);

					if (damage_result < GetAV(tiny, ActorValue::kStamina)) {
						return; // Fully protect against size-related damage
					}
				}
				if (apply_damage) {
					SizeHitEffects::PerformInjuryDebuff(giant, tiny, damage_result * bbmult, random);

					ModVulnerability(giant, tiny, damage_result);
					InflictSizeDamage(giant, tiny, damage_result);
					const float crush_difference = SMT
						? GetPostTinyCalamityShrinkDifference(tiny, size_difference)
						: size_difference;
					CrushCheck(giant, tiny, crush_difference, crush_threshold, Cause);
				}
			}
		}
	}

	void CollisionDamage::CrushCheck(Actor* giant, Actor* tiny, float size_difference, float crush_threshold, DamageSource Cause) {
		bool CanBeCrushed = (
			GetAV(tiny, ActorValue::kHealth) <= 1.0f ||
			tiny->IsDead()
		);
		
		if (CanBeCrushed) {
			if (size_difference > Action_Crush * crush_threshold && CrushManager::CanCrush(giant, tiny)) {
				ModSizeExperience_Crush(giant, tiny, true);

				if (!tiny->IsDead()) {
					if (IsGiant(tiny)) {
						AdvanceQuestProgression(giant, tiny, QuestStage::Giant, 1, false);
					} else {
						AdvanceQuestProgression(giant, tiny, QuestStage::Crushing, 1, false);
					}
				} else {
					AdvanceQuestProgression(giant, tiny, QuestStage::Crushing, 0.25f, false);
				}
				SetReanimatedState(tiny);

				CrushBonuses(giant, tiny);
				ReportDeath(giant, tiny, Cause);
				if (!Config::General.bLessGore) {
					auto node = find_node(giant, GetDeathNodeName(Cause));
					if (!IsMechanical(tiny)) {
						PlayCrushSound(giant, node, StrongGore(Cause), get_corrected_scale(tiny)); // Run Crush Sound task that will determine which exact type of crushing audio to play
					} 
				}

				SetBetweenBreasts(tiny, false);
				SetBeingHeld(tiny, false);

				CrushManager::Crush(giant, tiny);
			}
		}
	}
}
