#include "Managers/Impact.hpp"

#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"

#include "Managers/GTSSizeManager.hpp"
#include "Managers/ModEvent.hpp"

#include "Config/Config.hpp"

using namespace GTS;

namespace {

	bool CanDoImpact(Actor* actor, FootEvent kind) { // This function is needed to prevent sound spam from followers at large sizes
		if (IsTeammate(actor) && !actor->IsPlayerRef()) {
			if (get_visual_scale(actor) < 6.0f) {
				return true;
			}
			if (kind == FootEvent::Right) {
				if (IsActionOnCooldown(actor, CooldownSource::Footstep_Right)) {
					return false;
				}
				ApplyActionCooldown(actor, CooldownSource::Footstep_Right);
			} else if (kind == FootEvent::Left) {
				if (IsActionOnCooldown(actor, CooldownSource::Footstep_Left)) {
					return false;
				}
				ApplyActionCooldown(actor, CooldownSource::Footstep_Left);
			}
		} else {
			return true;
		}
		return true;
	}

	FootEvent get_foot_kind(Actor* actor, std::string_view tag) {
		GTS_PROFILE_SCOPE("Impact: GetFootKind");

		const bool hugging = AnimationVars::General::IsFollower(actor);
		const bool is_jumping = AnimationVars::Other::IsJumping(actor);
		const bool in_air = actor->IsInMidair();
		const bool is_sprinting = actor->AsActorState()->IsSprinting();
		const bool allow_footstep = !is_jumping || hugging;
		const bool has_foot = tag.contains("Foot");
		const bool is_sprint_tag = tag.contains("Sprint");
	
		// Hugging is needed to fix missing footsteps once we do friendly release
		// Footsteps aren't seen by the dll without it (because actor is in air)

		// Skip regular foot events if sprinting
		if (allow_footstep && has_foot && tag.contains("Left") && (!is_sprinting || is_sprint_tag)) {
			return FootEvent::Left;
		}
		if (allow_footstep && has_foot && tag.contains("Right") && (!is_sprinting || is_sprint_tag)) {
			return FootEvent::Right;
		}
		if (allow_footstep && !is_sprinting && has_foot && tag.contains("Front")) {
			return FootEvent::Front;
		}
		if (allow_footstep && !is_sprinting && has_foot && tag.contains("Back")) {
			return FootEvent::Back;
		}
		if (!in_air && tag.contains("Jump") && (tag.contains("Down") || tag.contains("Land"))) {
			return FootEvent::JumpLand;
		}
		return FootEvent::Unknown;
	}

	std::vector<NiAVObject*> get_landing_nodes(Actor* actor, const FootEvent& foot_kind) {
		GTS_PROFILE_SCOPE("Impact: GetLandingNodes");
		std::vector<NiAVObject*> results;
		results.reserve(2);
		const std::string_view left_foot = "NPC L Foot [Lft ]";
		const std::string_view right_foot = "NPC R Foot [Rft ]";
		const std::string_view left_arm = "NPC L Hand [LHnd]";
		const std::string_view right_arm = "NPC R Hand [RHnd]";

		NiAVObject* result;
		switch (foot_kind) {
			case FootEvent::Left:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Right:
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Front:
				result = find_node(actor, left_arm);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_arm);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Back:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::JumpLand:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
		}
		return results;
	}

	void DoExplosionAndSound(Actor* actor, FootEvent kind) {
		Impact impact_data = Impact {
			.actor = actor,
			.kind = kind,
			.scale = get_visual_scale(actor),
			.modifier = 1.0f,
			.nodes = get_landing_nodes(actor, kind),
		};

		EventDispatcher::DoOnImpact(impact_data); // Calls Explosions and sounds. A Must.
	}

	void ApplyPerkBonuses(Actor* actor, float& launch, float& radius) {
		if (actor->AsActorState()->IsWalking()) {
			launch = 0.8f;
		}
		// We already *= sneak damage by 0.70 inside DoSizeDamage() function, no point in doing it here
		if (actor->AsActorState()->IsSprinting()) {
			launch *= 1.150f;
			radius *= 1.175f;
			if (Runtime::HasPerkTeam(actor, Runtime::PERK.GTSPerkSprintDamageMult2)) {
				launch *= 1.250f;
				radius *= 1.275f;
			}
		}
	}

	DamageSource EventToSource(FootEvent kind) {
		DamageSource source = DamageSource::WalkLeft;
		if (kind == FootEvent::Right) {
			source = DamageSource::WalkRight;
		}
		return source;
	}

	void DoJumpLandEffects(Actor* actor) {
		auto& sizemanager = SizeManager::GetSingleton();
		float perk = GetPerkBonus_Basics(actor);
		float fallmod = GetFallModifier(actor);
		
		float damage = sizemanager.GetSizeAttribute(actor, SizeAttribute::JumpFall) * fallmod; // get jump damage boost
		bool EffectDelay = Config::General.bAlterPlayerGravity;
		double gravity = 1.0;
		
		if (EffectDelay && actor->GetCharController()) {
			gravity = std::clamp(actor->GetCharController()->gravity, 1.0f, 999999.0f);
		}

		const double EffectDelay_Gravity= static_cast<double>(EffectDelay ? Config::General.fAdditionalJumpEffectDelay_Gravity : 0.0f);
		const double EffectDelay_Normal = static_cast<double>(Config::General.fAdditionalJumpEffectDelay);
		
		std::string name = std::format("JumpLandT_{}", actor->formID);
		ActorHandle gianthandle = actor->CreateRefHandle();
		double Start = Time::WorldTimeElapsed();
		
		TaskManager::Run(name, [=](auto& progressData) { // Delay it a bit since it often happens in the air
			if (!gianthandle) {
				return false; // end task
			}
			auto giant = gianthandle.get().get();
			if (!giant) {
				return false;
			}
			double timepassed = Time::WorldTimeElapsed() - Start;
			float ClampedJump = std::clamp(fallmod, 1.0f, 2.0f);
			
			if (timepassed >= EffectDelay_Normal + (EffectDelay_Gravity * (gravity - 1.0))) {
				DoExplosionAndSound(giant, FootEvent::JumpLand);

				DoDamageEffect(giant, Damage_Jump_Default * damage, Radius_Jump_Default, 20, 0.25f, FootEvent::Left, 1.0f, DamageSource::CrushedLeft, true);
				DoDamageEffect(giant, Damage_Jump_Default * damage, Radius_Jump_Default, 20, 0.25f, FootEvent::Right, 1.0f, DamageSource::CrushedRight, true);

				DoLaunch(giant, 1.20f * perk * ClampedJump, 1.35f * fallmod, FootEvent::Left);
				DoLaunch(giant, 1.20f * perk * ClampedJump, 1.35f * fallmod, FootEvent::Right);
				return false;
			}
			return true;
		});
	}

	void DoDamageAndLaunch(Actor* actor, FootEvent kind, float launch, float radius) {
		if (kind != FootEvent::JumpLand) { // If just walking
			DoExplosionAndSound(actor, kind);
			DoDamageEffect(actor, Damage_Walk_Default, Radius_Walk_Default * radius, 25, 0.25f, kind, 1.25f, EventToSource(kind), true);
			DoLaunch(actor, 1.05f * launch, 2.6f * radius, kind);
		} else { // If jump landing
			DoJumpLandEffects(actor);
		}
	}
}

namespace GTS {

	void ImpactManager::HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		// Applied when Foot Events such as FootScuffLeft/FootScuffRight and FootLeft/FootRight are seen on Actors
		if (!a_event) {
			return;
		}

		GTS_PROFILE_SCOPE("Impact: HookProcessEvent");
		auto actor = a_event->actor.get().get();
		if (!actor) {
			return;
		}

		auto id = a_event->pad04;
		if (id == 10000001) {
			// .dll sends fake footstep events to fix missing foot sounds during some animations.
			// If it matches that number = we don't want to do anything. Done inside FootStepManager::PlayVanillaFootstepSounds function.
			return;
		}

		std::string_view tag_view = a_event->tag.c_str();
		std::string tag(tag_view);
		ModEventManager::GetSingleton().m_onfootstep.SendEvent(actor, tag);

		auto kind = get_foot_kind(actor, tag_view);
		if (kind == FootEvent::Unknown) {
			return;
		}

		if (CanDoImpact(actor, kind)) { // Prevents earrape and effect spam from followers when they're large
			float launch = 1.0f;
			float radius = 1.0f;

			ApplyPerkBonuses(actor, launch, radius);
			DoDamageAndLaunch(actor, kind, launch, radius); // Applies sounds as well
		}
	}
}
