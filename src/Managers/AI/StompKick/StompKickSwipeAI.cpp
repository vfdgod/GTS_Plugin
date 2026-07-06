#include "Managers/AI/StompKick/StompKickSwipeAI.hpp"
#include "Config/Config.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Stomp_Under.hpp"
#include "Utils/Actions/ActionUtils.hpp"
#include "Utils/Actor/AutoAimUtils.hpp"

using namespace GTS;

namespace {

	constexpr float MINIMUM_STOMP_DISTANCE = 50.0f;
	constexpr float MINIMUM_STOMP_SCALE_RATIO = 1.5f;
	constexpr float STOMP_ANGLE = 50;
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

	bool CanStomp(Actor* a_Pred, Actor* a_Prey) {

		if (a_Pred == a_Prey) {
			return false;
		}

		if (ProtectFollowers(a_Pred, a_Prey)) {
			return false;
		}

		if (!CanPerformActionOn(a_Pred, a_Prey, false)) {
			return false;
		}

		const float PredScale = get_visual_scale(a_Pred);
		const float SizeDiff = get_scale_difference(a_Pred, a_Prey, SizeType::VisualScale, true, false);

		float bonus = 1.0f;
		if (AnimationVars::Crawl::IsCrawling(a_Pred)) {
			bonus = 2.0f; // +100% stomp distance
		}

		if (!CanStompDead(a_Prey, SizeDiff)) { // We don't want the follower to be stuck stomping corpses that can't be crushed.
			return false;
		}

		float prey_distance = (a_Pred->GetPosition() - a_Prey->GetPosition()).Length();
		if (a_Pred->IsPlayerRef() && prey_distance <= (MINIMUM_STOMP_DISTANCE * PredScale * bonus) && SizeDiff < MINIMUM_STOMP_SCALE_RATIO) {
			return false;
		}

		if (prey_distance <= (MINIMUM_STOMP_DISTANCE * PredScale * bonus) && SizeDiff > MINIMUM_STOMP_SCALE_RATIO) { // We don't want the Stomp to be too close
			return true;
		}

		return false;
	}

	void Do_LightKick(Actor* pred) {
		const int idx = RandomIntWeighted(10, 10);
		AnimationManager::StartAnim(light_kicks.at(idx), pred);
	}

	void Do_HeavyKick(Actor* a_Performer) {
		int idx = RandomIntWeighted(10, 10, 10, 10);
		AnimationManager::StartAnim(heavy_kicks.at(idx), a_Performer);
	}

	void Do_LightSwipe(Actor* a_Performer) {
		int idx = RandomIntWeighted(10, 10);
		AnimationManager::StartAnim(light_kicks.at(idx), a_Performer);
	}

	void Do_HeavySwipe(Actor* a_Performer) {
		int idx = RandomIntWeighted(10, 10);
		AnimationManager::StartAnim(heavy_kicks.at(idx), a_Performer);
	}

	void Do_StrongStomp(Actor* a_Performer, Actor* a_Prey) {
		bool Left = AutoAim_SetUpDefaultSide(a_Performer);
		const bool UnderStomp = AnimationUnderStomp::AutoAim_And_DetermineStompType(a_Performer, Left, true);
		const std::string_view StompType_R = UnderStomp ? "UnderStompStrongRight" : "StrongStompRight";
		const std::string_view StompType_L = UnderStomp ? "UnderStompStrongLeft" : "StrongStompLeft";

		if (!Left) {
			AnimationManager::StartAnim(StompType_R, a_Performer);
		} else {
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
			AnimationManager::StartAnim(StompType_R, a_Performer);
		} else {
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
			AnimationManager::StartAnim(TrampleType_R, a_Performer);
		} else {
			AnimationManager::StartAnim(TrampleType_L, a_Performer);
		}
	}
}

namespace GTS {

	std::vector<Actor*> StompKickSwipeAI_FilterList(Actor* a_Pred, const std::vector<Actor*>& a_PotentialPrey) {
		if (!a_Pred) {
			return {};
		}

		auto CharacterController = a_Pred->GetCharController();
		if (!CharacterController) {
			return {};
		}

		auto preys = SelectTargetsInFront(a_Pred, a_PotentialPrey, a_PotentialPrey.size(), STOMP_ANGLE, true, [a_Pred](auto prey) {
			return CanStomp(a_Pred, prey);
		});
		return GetMaxActionableTinyCount(a_Pred, preys);
	}



	void StompAI_Start(Actor* a_Performer, Actor* a_Prey) {

		switch (RandomIntWeighted(10,10,10)) {

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

		switch (RandomIntWeighted(10, 10)) {

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
