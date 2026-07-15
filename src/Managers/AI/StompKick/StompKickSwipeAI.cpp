#include "Managers/AI/StompKick/StompKickSwipeAI.hpp"
#include "Config/Config.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Stomp_Under.hpp"
#include "Utils/Actions/ActionUtils.hpp"
#include "Utils/Actor/AutoAimUtils.hpp"
#include "Managers/Animation/StompAssist.hpp"

#include <algorithm>
#include <cmath>
#include <string_view>
#include <utility>

using namespace GTS;

namespace {

	constexpr float MINIMUM_STOMP_DISTANCE = 50.0f;
	constexpr float MINIMUM_STOMP_SCALE_RATIO = 1.5f;
	constexpr float STOMP_ANGLE = 50;
	constexpr float PI = 3.14159265358979323846f;
	//Light Kick/Swipe
	const std::vector<std::string> light_kicks = {
		"SwipeLight_Left",                  // 0
		"SwipeLight_Right",                 // 1
	};

	//Heavy Kick/Swipe, The low ones only work when standing
	const std::vector<std::string> heavy_kicks = {
		"SwipeHeavy_Left",                  // 0
		"SwipeHeavy_Right",                 // 1
		"StrongKick_Low_Right",             // 2
		"StrongKick_Low_Left",              // 3
	};

	bool ShouldLogStompKickAI(Actor* a_Pred) {
		return a_Pred && (a_Pred->IsPlayerRef() || IsTeammate(a_Pred));
	}

	const char* GetActorLogName(Actor* a_Actor) {
		if (!a_Actor) {
			return "<null>";
		}
		return a_Actor->GetDisplayFullName();
	}

	float GetForwardAngleToTarget(Actor* a_Pred, Actor* a_Prey) {
		if (!a_Pred || !a_Prey) {
			return 0.0f;
		}

		NiPoint3 preyDir = a_Prey->GetPosition() - a_Pred->GetPosition();
		preyDir.z = 0.0f;
		const float preyDirLength = preyDir.Length();
		if (preyDirLength <= 1e-4f) {
			return 0.0f;
		}

		RE::NiPoint3 forwardVector{ 0.0f, 1.0f, 0.0f };
		RE::NiPoint3 predDir = RotateAngleAxis(forwardVector, -a_Pred->data.angle.z, { 0.0f, 0.0f, 1.0f });
		predDir.z = 0.0f;
		const float predDirLength = predDir.Length();
		if (predDirLength <= 1e-4f) {
			return 180.0f;
		}

		const float dot = std::clamp(predDir.Dot(preyDir) / (predDirLength * preyDirLength), -1.0f, 1.0f);
		return std::acos(dot) * 180.0f / PI;
	}

	void LogStompKickReject(Actor* a_Pred, Actor* a_Prey, std::string_view a_Reason) {
		if (!ShouldLogStompKickAI(a_Pred)) {
			return;
		}

		logger::info(
			"[AI StompKick] reject reason={} pred={}({:08X}) prey={}({:08X})",
			a_Reason,
			GetActorLogName(a_Pred),
			a_Pred ? a_Pred->formID : 0,
			GetActorLogName(a_Prey),
			a_Prey ? a_Prey->formID : 0
		);
	}

	void LogStompKickCandidate(Actor* a_Pred, Actor* a_Prey, std::string_view a_Result, float a_PredScale, float a_SizeDiff, float a_Distance, float a_MaxDistance, float a_ForwardAngle) {
		if (!ShouldLogStompKickAI(a_Pred)) {
			return;
		}

		logger::info(
			"[AI StompKick] candidate result={} pred={}({:08X}) prey={}({:08X}) predScale={:.2f} sizeDiff={:.2f} distance={:.1f}/{:.1f} forwardAngle={:.1f}/{} crawling={} sneaking={}",
			a_Result,
			GetActorLogName(a_Pred),
			a_Pred ? a_Pred->formID : 0,
			GetActorLogName(a_Prey),
			a_Prey ? a_Prey->formID : 0,
			a_PredScale,
			a_SizeDiff,
			a_Distance,
			a_MaxDistance,
			a_ForwardAngle,
			STOMP_ANGLE,
			AnimationVars::Crawl::IsCrawling(a_Pred),
			a_Pred ? a_Pred->IsSneaking() : false
		);
	}

	void LogStompKickAnimation(Actor* a_Performer, std::string_view a_Trigger) {
		if (!ShouldLogStompKickAI(a_Performer)) {
			return;
		}

		logger::info(
			"[AI StompKick] start animation pred={}({:08X}) trigger={}",
			GetActorLogName(a_Performer),
			a_Performer ? a_Performer->formID : 0,
			a_Trigger
		);
	}

	bool ProtectFollowers(Actor* a_Pred, Actor* a_Prey) {
		bool NPC = Config::General.bProtectFollowers;
		bool Hostile = IsHostile(a_Pred, a_Prey);

		if (!a_Prey->IsPlayerRef() && !Hostile && NPC && (IsTeammate(a_Pred)) && (IsTeammate(a_Prey))) {
			return true; // Disallow NPC's to perform stomps on followers
		}

		return false;
	}

	bool CanStompDead(Actor* a_Prey, float a_SizeDifference) {
		if (a_Prey->IsDead() && a_SizeDifference < Action_Crush) {
			return false;
		}
		return true;
	}

	bool CanStomp(Actor* a_Pred, Actor* a_Prey, bool a_UseStompAssistFix) {

		if (!a_Pred || !a_Prey) {
			LogStompKickReject(a_Pred, a_Prey, "null_actor");
			return false;
		}

		if (a_Pred == a_Prey) {
			LogStompKickReject(a_Pred, a_Prey, "same_actor");
			return false;
		}

		if (ProtectFollowers(a_Pred, a_Prey)) {
			LogStompKickReject(a_Pred, a_Prey, "protected_follower");
			return false;
		}

		if (!CanPerformActionOn(a_Pred, a_Prey, false)) {
			LogStompKickReject(a_Pred, a_Prey, "cannot_perform_action_on");
			return false;
		}

		const float PredScale = get_visual_scale(a_Pred);
		const float SizeDiff = get_scale_difference(a_Pred, a_Prey, SizeType::VisualScale, true, false);

		float bonus = 1.0f;
		if (AnimationVars::Crawl::IsCrawling(a_Pred)) {
			bonus = 2.0f; // +100% stomp distance
		}

		if (!CanStompDead(a_Prey, SizeDiff)) { // We don't want the follower to be stuck stomping corpses that can't be crushed.
			LogStompKickCandidate(a_Pred, a_Prey, "fail_dead_below_crush_size", PredScale, SizeDiff, 0.0f, 0.0f, GetForwardAngleToTarget(a_Pred, a_Prey));
			return false;
		}

		float prey_distance = (a_Pred->GetPosition() - a_Prey->GetPosition()).Length();
		float max_distance = MINIMUM_STOMP_DISTANCE * PredScale * bonus;
		float forward_angle = GetForwardAngleToTarget(a_Pred, a_Prey);
		if (a_Pred->IsPlayerRef() && prey_distance <= (MINIMUM_STOMP_DISTANCE * PredScale * bonus) && SizeDiff < MINIMUM_STOMP_SCALE_RATIO) {
			LogStompKickCandidate(a_Pred, a_Prey, "fail_player_size_diff", PredScale, SizeDiff, prey_distance, max_distance, forward_angle);
			return false;
		}

		// AI stomp fix only: nearby trigger uses Stomp Assist radius/size (no facing cone).
		// Kicks always use the classic distance/size path below + front cone filter.
		if (a_UseStompAssistFix) {
			const float assistDistance = GetStompAssistSearchDistance(a_Pred) * bonus;
			const float assistSizeThreshold = GetStompAssistSizeThreshold();
			const float checkDistance = assistDistance > 0.0f ? assistDistance : max_distance;

			if (prey_distance <= checkDistance && SizeDiff >= assistSizeThreshold) {
				LogStompKickCandidate(a_Pred, a_Prey, "pass_stomp_assist_nearby", PredScale, SizeDiff, prey_distance, checkDistance, forward_angle);
				return true;
			}

			const std::string_view reason = prey_distance > checkDistance && SizeDiff < assistSizeThreshold ?
				"fail_distance_and_size_diff" :
				(prey_distance > checkDistance ? "fail_distance" : "fail_size_diff");
			LogStompKickCandidate(a_Pred, a_Prey, reason, PredScale, SizeDiff, prey_distance, checkDistance, forward_angle);
			return false;
		}

		if (prey_distance <= (MINIMUM_STOMP_DISTANCE * PredScale * bonus) && SizeDiff > MINIMUM_STOMP_SCALE_RATIO) {
			LogStompKickCandidate(a_Pred, a_Prey, "pass_range_size_before_front_cone", PredScale, SizeDiff, prey_distance, max_distance, forward_angle);
			return true;
		}

		const std::string_view reason = prey_distance > max_distance && SizeDiff <= MINIMUM_STOMP_SCALE_RATIO ?
			"fail_distance_and_size_diff" :
			(prey_distance > max_distance ? "fail_distance" : "fail_size_diff");
		LogStompKickCandidate(a_Pred, a_Prey, reason, PredScale, SizeDiff, prey_distance, max_distance, forward_angle);
		return false;
	}

	void Do_LightKick(Actor* pred) {
		const int idx = RandomIntWeighted(10, 10);
		const auto& trigger = light_kicks.at(idx);
		LogStompKickAnimation(pred, trigger);
		AnimationManager::StartAnim(trigger, pred);
	}

	void Do_HeavyKick(Actor* a_Performer) {
		int idx = RandomIntWeighted(10, 10, 10, 10);
		const auto& trigger = heavy_kicks.at(idx);
		LogStompKickAnimation(a_Performer, trigger);
		AnimationManager::StartAnim(trigger, a_Performer);
	}

	void Do_LightSwipe(Actor* a_Performer) {
		int idx = RandomIntWeighted(10, 10);
		const auto& trigger = light_kicks.at(idx);
		LogStompKickAnimation(a_Performer, trigger);
		AnimationManager::StartAnim(trigger, a_Performer);
	}

	void Do_HeavySwipe(Actor* a_Performer) {
		int idx = RandomIntWeighted(10, 10);
		const auto& trigger = heavy_kicks.at(idx);
		LogStompKickAnimation(a_Performer, trigger);
		AnimationManager::StartAnim(trigger, a_Performer);
	}

	void Do_StrongStomp(Actor* a_Performer, Actor* a_Prey) {
		bool Left = AutoAim_SetUpDefaultSide(a_Performer);
		const bool UnderStomp = AnimationUnderStomp::AutoAim_And_DetermineStompType(a_Performer, Left, true);
		const std::string_view StompType_R = UnderStomp ? "UnderStompStrongRight" : "StrongStompRight";
		const std::string_view StompType_L = UnderStomp ? "UnderStompStrongLeft" : "StrongStompLeft";

		if (!Left) {
			if (Config::AI.bStompKickAutoAimFix) {
				TryStompAssist(a_Performer, true, StompAssistAction::Strong, a_Prey);
			}
			LogStompKickAnimation(a_Performer, StompType_R);
			AnimationManager::StartAnim(StompType_R, a_Performer);
		} else {
			if (Config::AI.bStompKickAutoAimFix) {
				TryStompAssist(a_Performer, false, StompAssistAction::Strong, a_Prey);
			}
			LogStompKickAnimation(a_Performer, StompType_L);
			AnimationManager::StartAnim(StompType_L, a_Performer);
		}
	}

	void Do_LightStomp(Actor* a_Performer, Actor* a_Prey) {
		bool Left = AutoAim_SetUpDefaultSide(a_Performer);
		Utils_UpdateHighHeelBlend(a_Performer, false);
		const bool UnderStomp = AnimationUnderStomp::AutoAim_And_DetermineStompType(a_Performer, Left);
		const std::string_view StompType_R = UnderStomp ? "UnderStompRight" : "StompRight";
		const std::string_view StompType_L = UnderStomp ? "UnderStompLeft" : "StompLeft";

		if (!Left) {
			if (Config::AI.bStompKickAutoAimFix) {
				TryStompAssist(a_Performer, true, StompAssistAction::Normal, a_Prey);
			}
			LogStompKickAnimation(a_Performer, StompType_R);
			AnimationManager::StartAnim(StompType_R, a_Performer);
		} else {
			if (Config::AI.bStompKickAutoAimFix) {
				TryStompAssist(a_Performer, false, StompAssistAction::Normal, a_Prey);
			}
			LogStompKickAnimation(a_Performer, StompType_L);
			AnimationManager::StartAnim(StompType_L, a_Performer);
		}
	}

	void Do_Tramples(Actor* a_Performer, Actor* a_Prey) {
		bool Left = AutoAim_SetUpDefaultSide(a_Performer);
		bool UnderTrample = AnimationUnderStomp::AutoAim_And_DetermineStompType(a_Performer, Left);
		const std::string_view TrampleType_L = UnderTrample ? "UnderTrampleL" : "TrampleL";
		const std::string_view TrampleType_R = UnderTrample ? "UnderTrampleR" : "TrampleR";

		Utils_UpdateHighHeelBlend(a_Performer, false);
		if (!Left) {
			if (Config::AI.bStompKickAutoAimFix && !UnderTrample) {
				TryStompAssist(a_Performer, true, StompAssistAction::Trample, a_Prey);
			}
			LogStompKickAnimation(a_Performer, TrampleType_R);
			AnimationManager::StartAnim(TrampleType_R, a_Performer);
		} else {
			if (Config::AI.bStompKickAutoAimFix && !UnderTrample) {
				TryStompAssist(a_Performer, false, StompAssistAction::Trample, a_Prey);
			}
			LogStompKickAnimation(a_Performer, TrampleType_L);
			AnimationManager::StartAnim(TrampleType_L, a_Performer);
		}
	}
}

namespace GTS {

	std::vector<Actor*> StompKickSwipeAI_FilterList(Actor* a_Pred, const std::vector<Actor*>& a_PotentialPrey, bool a_UseStompAssistFix) {
		if (!a_Pred) {
			return {};
		}

		auto CharacterController = a_Pred->GetCharController();
		if (!CharacterController) {
			LogStompKickReject(a_Pred, nullptr, "no_character_controller");
			return {};
		}

		// Only stomps may use assist-nearby when AI fix is on; kicks always pass false.
		const bool useStompAssistFix = a_UseStompAssistFix && Config::AI.bStompKickAutoAimFix;
		std::size_t rangeAndSizePassCount = 0;
		std::vector<Actor*> preys;

		if (useStompAssistFix) {
			std::vector<std::pair<Actor*, float>> candidates;
			candidates.reserve(a_PotentialPrey.size());

			for (auto prey : a_PotentialPrey) {
				if (!CanStomp(a_Pred, prey, true)) {
					continue;
				}

				++rangeAndSizePassCount;
				const auto preyOffset = prey->GetPosition() - a_Pred->GetPosition();
				candidates.emplace_back(prey, preyOffset.Length());
			}

			std::ranges::sort(candidates, std::ranges::less{}, &std::pair<Actor*, float>::second);
			preys.reserve(candidates.size());
			for (const auto& candidate : candidates) {
				preys.push_back(candidate.first);
			}
		}
		else {
			preys = SelectTargetsInFront(a_Pred, a_PotentialPrey, a_PotentialPrey.size(), STOMP_ANGLE, true, [a_Pred, &rangeAndSizePassCount](auto prey) {
				const bool canStomp = CanStomp(a_Pred, prey, false);
				if (canStomp) {
					++rangeAndSizePassCount;
				}
				return canStomp;
			});
		}
		auto finalPreys = GetMaxActionableTinyCount(a_Pred, preys);

		if (ShouldLogStompKickAI(a_Pred)) {
			logger::info(
				"[AI StompKick] filter result pred={}({:08X}) potential={} range_size_pass={} position_selected={} final={} coneAngle={} baseDistance={} minSizeDiff={} stompAssistFix={} assistRadius={:.1f} assistSize={:.1f}",
				GetActorLogName(a_Pred),
				a_Pred->formID,
				a_PotentialPrey.size(),
				rangeAndSizePassCount,
				preys.size(),
				finalPreys.size(),
				STOMP_ANGLE,
				MINIMUM_STOMP_DISTANCE,
				MINIMUM_STOMP_SCALE_RATIO,
				useStompAssistFix,
				useStompAssistFix ? GetStompAssistSearchDistance(a_Pred) : 0.0f,
				useStompAssistFix ? GetStompAssistSizeThreshold() : MINIMUM_STOMP_SCALE_RATIO
			);
		}

		return finalPreys;
	}



	void StompAI_Start(Actor* a_Performer, Actor* a_Prey) {

		const int stompType = RandomIntWeighted(10,10,10);
		if (ShouldLogStompKickAI(a_Performer)) {
			logger::info(
				"[AI StompKick] start stomp pred={}({:08X}) prey={}({:08X}) roll={} meaning={}",
				GetActorLogName(a_Performer),
				a_Performer ? a_Performer->formID : 0,
				GetActorLogName(a_Prey),
				a_Prey ? a_Prey->formID : 0,
				stompType,
				stompType == 0 ? "light_stomp" : (stompType == 1 ? "strong_stomp" : "trample_or_light_if_sneak_crawl")
			);
		}

		switch (stompType) {

			case 0: {
				Do_LightStomp(a_Performer, a_Prey);
				return;
			}
			case 1:{
				Do_StrongStomp(a_Performer, a_Prey);
				return;
			}
			case 2: {

				if (!AnimationVars::Crawl::IsCrawling(a_Performer) && !a_Performer->IsSneaking()) {
					Do_Tramples(a_Performer, a_Prey);
				}
				else {
					Do_LightStomp(a_Performer, a_Prey);
				}
			}
			default: {}
		}
	}

	void KickSwipeAI_Start(Actor* a_Performer) {

		Utils_UpdateHighHeelBlend(a_Performer, false);
		const bool Crawling = AnimationVars::Crawl::IsCrawling(a_Performer);

		const int kickType = RandomIntWeighted(10, 10);
		if (ShouldLogStompKickAI(a_Performer)) {
			logger::info(
				"[AI StompKick] start kick_swipe pred={}({:08X}) roll={} crawling={} meaning={}",
				GetActorLogName(a_Performer),
				a_Performer ? a_Performer->formID : 0,
				kickType,
				Crawling,
				kickType == 0 ? "light" : "heavy"
			);
		}

		switch (kickType) {

			case 0: {

				if (Crawling) {
					Do_LightSwipe(a_Performer);
				}
				else {
					Do_LightKick(a_Performer);
				}
				return;
			}
			case 1:{
				if (Crawling) {
					Do_HeavySwipe(a_Performer);
				}
				else {
					Do_HeavyKick(a_Performer);
				}
			}
			default: {}
		}
	}
}
