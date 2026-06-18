
#include "Managers/Animation/Custom_Events_ModSupport.hpp"
#include "Managers/Animation/TinyCalamity_Instakill.hpp"
#include "Managers/Animation/Sneak_Slam_FingerGrind.hpp"
#include "Managers/Animation/Stomp_Under_FullBody.hpp"
#include "Managers/Animation/TinyCalamity_Shrink.hpp"
#include "Managers/Animation/Stomp_Under_Strong.hpp"
#include "Managers/Animation/Sneak_Slam_Strong.hpp"
#include "Managers/Animation/Stomp_Under_Slam.hpp"
#include "Managers/Animation/Stomp_Under_Butt.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/FurnitureAnimations.hpp"

#include "Config/Config.hpp"

#include "Managers/Animation/CleavageStrangle.hpp"
#include "Managers/Animation/Grab_Sneak_Vore.hpp"
#include "Managers/Animation/Sneak_KneeCrush.hpp"
#include "Managers/Animation/CleavageEvents.hpp"
#include "Managers/Animation/CleavageState.hpp"
#include "Managers/Animation/Vore_Standing.hpp"
#include "Managers/Animation/ThighSandwich.hpp"
#include "Managers/Animation/ThighSandwichPart2.hpp"
#include "Managers/Animation/Sneak_Swipes.hpp"
#include "Managers/Animation/RandomGrowth.hpp"
#include "Managers/Animation/Stomp_Normal.hpp"
#include "Managers/Animation/Stomp_Strong.hpp"
#include "Managers/Animation/Stomp_Under.hpp"
#include "Managers/Animation/FootTrample.hpp"
#include "Managers/Animation/Grab_Attack.hpp"

#include "Managers/Animation/Grab_Throw.hpp"
#include "Managers/Animation/Grab_Play_Events.hpp"
#include "Managers/Animation/Grab_Play.hpp"
#include "Managers/Animation/Sneak_Slam.hpp"
#include "Managers/Animation/Vore_Sneak.hpp"
#include "Managers/Animation/Vore_Crawl.hpp"
#include "Managers/Animation/ThighCrush.hpp"
#include "Managers/Animation/Grab_Vore.hpp"
#include "Managers/Animation/ButtCrush.hpp"
#include "Managers/Animation/BoobCrush.hpp"
#include "Managers/Animation/FootGrind.hpp"
#include "Managers/Animation/HugShrink.hpp"
#include "Managers/Animation/Crawling.hpp"
#include "Managers/Animation/HugHeal.hpp"
#include "Managers/Animation/Proning.hpp"
#include "Managers/Animation/Growth.hpp"
#include "Managers/Animation/Shrink.hpp"
#include "Managers/Animation/Kicks.hpp"
#include "Managers/Animation/Grab.hpp"

#include "Managers/Perks/PerkHandler.hpp"
#include "Utils/Actions/InputFunctions.hpp"

namespace GTS {

	AnimationEventData::AnimationEventData(Actor& giant, TESObjectREFR* tiny) : giant(giant), tiny(tiny) {}

	AnimationEvent::AnimationEvent(const std::function<void(AnimationEventData&)>& a_callback, const std::string& a_group) : callback(a_callback), group(a_group) {}

	TriggerData::TriggerData(const std::vector< std::string_view>& behavors,  std::string_view group) : behavors({}), group(group) {
		for (auto& sv: behavors) {
			this->behavors.emplace_back(sv);
		}
	}

	std::string AnimationManager::DebugName() {
		return "::AnimationManager";
	}

	void AnimationManager::DataReady() {
		AnimationStomp::RegisterEvents();
		AnimationStomp::RegisterTriggers();

		AnimationStrongStomp::RegisterEvents();
		AnimationStrongStomp::RegisterTriggers();

		AnimationUnderStomp::RegisterEvents();
		AnimationUnderStomp::RegisterTriggers();

		AnimationUnderStompStrong::RegisterEvents();
		AnimationUnderStompStrong::RegisterTriggers();

		AnimationUnderStompFullBody::RegisterEvents();
		AnimationUnderStompSlam::RegisterEvents();
		AnimationUnderStompButt::RegisterEvents();

		AnimationThighSandwich::RegisterEvents();
		AnimationThighSandwich::RegisterTriggers();

		AnimationThighSandwich_P2::RegisterEvents();
		AnimationThighSandwich_P2::RegisterTriggers();

		AnimationThighCrush::RegisterEvents();
		AnimationThighCrush::RegisterTriggers();

		AnimationCrawling::RegisterEvents();
		AnimationCrawling::RegisterTriggers();

		Animation_VoreStanding::RegisterEvents();
		Animation_VoreStanding::RegisterTriggers();

		Animation_VoreCrawl::RegisterEvents();
		Animation_VoreSneak::RegisterEvents();

		Animation_SneakSwipes::RegisterEvents();
		Animation_SneakSlam::RegisterEvents();
		Animation_SneakSlam_Strong::RegisterEvents();
		Animation_SneakSlam_FingerGrind::RegisterEvents();
		Animation_SneakSlam_FingerGrind::RegisterTriggers();

		Animation_Cleavage::RegisterEvents();
		Animation_Cleavage::RegisterTriggers();

		Animation_CleavageEvents::RegisterEvents();

		Animation_CleavageStrangle::RegisterEvents();
		Animation_CleavageStrangle::RegisterTriggers();

		Animation_TinyCalamity::RegisterEvents();
		Animation_TinyCalamity::RegisterTriggers();

		Animation_TinyCalamity_InstaKill::RegisterEvents();
		Animation_TinyCalamity_InstaKill::RegisterTriggers();

		AnimationButtCrush::RegisterEvents();
		AnimationButtCrush::RegisterTriggers();

		AnimationSneakCrush::RegisterEvents();

		AnimationBoobCrush::RegisterEvents();

		Animation_GrabSneak_Vore::RegisterEvents();
		Animation_GrabVore::RegisterEvents();
		Animation_GrabThrow::RegisterEvents();
		Animation_GrabAttack::RegisterEvents();
		Animation_GrabPlay_Events::RegisterEvents();
		Animation_GrabPlay::RegisterTriggers();
		
		Animation_RandomGrowth::RegisterEvents();
		Animation_RandomGrowth::RegisterTriggers();

		AnimationFootGrind::RegisterEvents();
		AnimationFootGrind::RegisterTriggers();

		AnimationFootTrample::RegisterEvents();
		AnimationFootTrample::RegisterTriggers();

		Animation_ModSupport::RegisterEvents();
		Animation_ModSupport::RegisterTriggers();

		AnimationProning::RegisterEvents();
		AnimationProning::RegisterTriggers();

		AnimationGrowth::RegisterEvents();
		AnimationGrowth::RegisterTriggers();

		AnimationShrink::RegisterEvents();
		AnimationShrink::RegisterTriggers();

		InputFunctions::RegisterEvents();

		AnimationKicks::RegisterEvents();
		AnimationKicks::RegisterTriggers();

		HugShrink::RegisterEvents();
		HugShrink::RegisterTriggers();

		HugHeal::RegisterEvents();
		HugHeal::RegisterTriggers();

		Grab::RegisterEvents();
		Grab::RegisterTriggers();

		FurnitureAnimations::RegisterEvents();
	}

	void AnimationManager::Update() {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			UpdateGravity(player);
		}
	}

	void AnimationManager::Reset() {
		this->data.clear();
	}

	void AnimationManager::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	float AnimationManager::GetHighHeelSpeed(Actor* actor) {
		float Speed = 1.0f;
		auto& AnimMgr = AnimationManager::GetSingleton();

		if (auto it = AnimMgr.data.find(actor); it != AnimMgr.data.end()) {
			for (auto& data : it->second | std::views::values) {
				Speed *= data.HHspeed;
			}
		}

		return Speed;
	}

	float AnimationManager::GetBonusAnimationSpeed(Actor* actor) {
		float totalSpeed = 1.0f;
		auto& AnimMgr = AnimationManager::GetSingleton();

		if (auto it = AnimMgr.data.find(actor); it != AnimMgr.data.end()) {
			for (auto& data : it->second | std::views::values) {
				totalSpeed *= data.animSpeed;
			}
		}

		return totalSpeed;
	}

	void AnimationManager::AdjustAnimSpeed(float bonus) {

		const auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}

		auto& AnimMgr = AnimationManager::GetSingleton();
		auto it = AnimMgr.data.find(player);
		if (it == AnimMgr.data.end()) {
			return;
		}

		const float animation_slowdown = GetAnimationSlowdown(player);
		const bool isBreastStrangling = AnimationVars::Cleavage::IsBoobsDoting(player);
		const float min = isBreastStrangling ? 0.50f : 0.33f;
		const float max = isBreastStrangling ? 1.75f : 3.0f;

		for (auto& data : it->second | std::views::values) {
			if (data.canEditAnimSpeed) {
				data.animSpeed += bonus * animation_slowdown;
			}
			data.animSpeed = std::clamp(data.animSpeed, min, max);
		}
	}

	float AnimationManager::GetAnimSpeed(Actor* actor) {

		float speed = 1.0f;

		if (!Config::General.bDynamicAnimspeed) {
			return 1.0f;
		}

		if (actor) {

			auto saved_data = GTS::Persistent::GetActorData(actor);
			if (saved_data) {
				if (saved_data->fAnimSpeed > 0.0f) {
					speed *= saved_data->fAnimSpeed;
				}
			}

			const auto& AnimMgr = AnimationManager::GetSingleton();
			const auto& AnimData = AnimMgr.data;
			float totalSpeed = 1.0f;

			if (auto it = AnimData.find(actor); it != AnimData.end()) {
				for (const auto& data : it->second | std::views::values) {
					totalSpeed *= data.animSpeed;
				}
				speed *= totalSpeed;
			}
		}
		return speed;
	}

	void AnimationManager::RegisterEvent( std::string_view name,  std::string_view group, std::function<void(AnimationEventData&)> func) {
		auto& callbacks = AnimationManager::GetSingleton().eventCallbacks[std::string(name)];
		if (std::ranges::any_of(callbacks, [group](const AnimationEvent& event) {
			return event.group == group;
		})) {
			logger::warn("Duplicate animation event registration ignored: {} ({})", name, group);
			return;
		}

		callbacks.emplace_back(func, std::string(group));
		//log::info("Registering Event: Name {}, Group {}", name, group);
	}

	void AnimationManager::RegisterTrigger( std::string_view trigger,  std::string_view group,  std::string_view behavior) {
		AnimationManager::RegisterTriggerWithStages(trigger, group, {behavior});
		//log::info("Registering Trigger: {}, Group {}, Behavior {}", trigger, group, behavior);
	}

	void AnimationManager::RegisterTriggerWithStages( std::string_view trigger,  std::string_view group,  std::vector< std::string_view> behaviors) {
		if (!behaviors.empty()) {
			AnimationManager::GetSingleton().triggers.try_emplace(std::string(trigger), behaviors, group);
			//log::info("Registering Trigger With Stages: {}, Group {}", trigger, group);
		}
	}


	void AnimationManager::StartAnim( std::string_view trigger, Actor& giant) {
		AnimationManager::StartAnim(trigger, giant, nullptr);
	}

	void AnimationManager::StartAnim( std::string_view trigger, Actor* giant) {
		if (giant) {
			AnimationManager::StartAnim(trigger, *giant);
			//log::info("Starting Trigger {} for {}", trigger, giant->GetDisplayFullName());
		}
	}

	void AnimationManager::StartAnim(std::string_view trigger, Actor& giant, TESObjectREFR* tiny) {

		if (AnimationVars::General::IsTransitioning(&giant)) {
			return;
		}

		if (giant.IsPlayerRef()) {
			if (IsFirstPerson() || State::IsInRaceMenu()) { 
				//Time::WorldTimeElapsed() > 1.0
				//ForceThirdPerson(&giant);
				// It kinda works in fp that way, but it introduces some issues with animations such as Hugs and Butt Crush.
				// Better to wait for full support someday
				return; // Don't start animations in FP, it's not supported.
			}
		}

		auto& me = AnimationManager::GetSingleton();
		auto triggerIt = me.triggers.find(std::string(trigger));
		if (triggerIt == me.triggers.end()) {
			logger::error("Requested play of unknown animation named: {}", trigger);
			return;
		}

		auto& behavorToPlay = triggerIt->second;
		auto& group = behavorToPlay.group;
		auto& actorData = me.data.try_emplace(&giant).first->second;
		actorData.try_emplace(group, giant, tiny);

		//log::info("Playing Trigger {} for {}", trigger, giant.GetDisplayFullName());
		//log::info("Playing {}", behavorToPlay.behavors[0]);
		giant.NotifyAnimationGraph(behavorToPlay.behavors[0]);

		PerkHandler::UpdatePerkValues(&giant, PerkUpdate::Perk_Acceleration); // Currently used for Anim Speed buff only
	}

	void AnimationManager::ResetAnimationSpeedData(Actor* actor) {
		auto& me = AnimationManager::GetSingleton();
		if (auto it = me.data.find(actor); it != me.data.end()) {
			for (auto& data : it->second | std::views::values) {
				data.animSpeed = 1.0f;
				data.canEditAnimSpeed = false;
				data.stage = 0;
			}
		}
	}



	void AnimationManager::StartAnim(std::string_view trigger, Actor* giant, TESObjectREFR* tiny) {
		if (giant) {
			AnimationManager::StartAnim(trigger, *giant, tiny);
		}
	}

	void AnimationManager::NextAnim(std::string_view trigger, Actor& giant) {
		auto& me = AnimationManager::GetSingleton();
		auto triggerIt = me.triggers.find(std::string(trigger));
		if (triggerIt == me.triggers.end()) {
			return;
		}

		auto& behavorToPlay = triggerIt->second;
		auto actorDataIt = me.data.find(&giant);
		if (actorDataIt == me.data.end()) {
			return;
		}

		auto eventDataIt = actorDataIt->second.find(behavorToPlay.group);
		if (eventDataIt == actorDataIt->second.end()) {
			return;
		}

		const std::size_t currentTrigger = eventDataIt->second.currentTrigger;
		if (currentTrigger < behavorToPlay.behavors.size()) {
			giant.NotifyAnimationGraph(behavorToPlay.behavors[currentTrigger]);
		}
	}
	void AnimationManager::NextAnim(std::string_view trigger, Actor* giant) {
		if (giant) {
			AnimationManager::NextAnim(trigger, *giant);
		}
	}

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		try {
			if (actor) {
				const std::string eventTag(tag);

				auto callbacks = this->eventCallbacks.find(eventTag);
				if (callbacks == this->eventCallbacks.end()) {
					return;
				}

				// If data doesn't exist then insert with default
				auto& actorData = this->data.try_emplace(actor).first->second;

				for (auto& animToPlay : callbacks->second) {
					const auto& group = animToPlay.group;
					auto actorDataIt = actorData.find(group);

					if (actorDataIt == actorData.end()) {
						if (callbacks->second.size() > 1) {
							continue;
						}

						// Unique tags preserve the legacy behavior: create event data on demand.
						actorDataIt = actorData.try_emplace(group, *actor, nullptr).first;
					}

					auto& actdata = actorDataIt->second;
					animToPlay.callback(actdata);

					// If the stage is 0 after an anim has been played then
					//   delete this data so that we can reset for the next anim
					if (actdata.stage == 0) {
						actorData.erase(group);
					}
				}
			}
		}
		catch (const std::out_of_range&) {}
	}

	// Get the current stage of an animation group
	std::size_t AnimationManager::GetStage(Actor& actor,  std::string_view group) {
		auto& me = AnimationManager::GetSingleton();
		auto actorDataIt = me.data.find(&actor);
		if (actorDataIt == me.data.end()) {
			return 0;
		}

		auto eventDataIt = actorDataIt->second.find(std::string(group));
		if (eventDataIt == actorDataIt->second.end()) {
			return 0;
		}

		return eventDataIt->second.stage;
	}

	std::size_t AnimationManager::GetStage(Actor* actor,  std::string_view group) {
		if (actor) {
			return AnimationManager::GetStage(*actor, group);
		}
		else {
			return 0;
		}
	}

	bool AnimationManager::HHDisabled(Actor& actor) {
		auto& me = AnimationManager::GetSingleton();

		if (!IsHumanoid(&actor) || me.data.empty()) {
			return false;
		}

		auto it = me.data.find(&actor);
		if (it == me.data.end()) {
			return false;
		}

		return std::ranges::any_of(it->second | std::views::values, [](const auto& data) {
			return data.disableHH;
		});
	}

	bool AnimationManager::HHDisabled(Actor* actor) {
		if (actor) {
			return AnimationManager::HHDisabled(*actor);
		}
		else {
			return false;
		}
	}

	void AnimationManager::UpdateGravity(Actor* actor) {
		static Timer GravityTimer = Timer(0.33);
		if (GravityTimer.ShouldRunFrame()) {
			auto Controller = actor->GetCharController();
			if (Controller) {
				bool Enabled = Config::General.bAlterPlayerGravity;
				float size = get_visual_scale(actor);

				float new_gravity = Enabled ? 1.0f * std::sqrt(size) : 1.0f; 
				Controller->gravity = new_gravity;
			}
		}
	}
}
