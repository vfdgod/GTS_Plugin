#include "Managers/Animation/Controllers/ThighCrushController.hpp"
#include "Managers/GTSSizeManager.hpp"
#include "Managers/HighHeel.hpp"

namespace {

	constexpr float MINIMUM_THIGH_DISTANCE = 58.0f;
	constexpr float THIGH_ANGLE = 75;
	constexpr float PI = std::numbers::pi_v<float>;

}

namespace GTS {

	std::vector<Actor*> ThighCrushController::GetThighTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (!pred) {
			return {};
		}
		auto charController = pred->GetCharController();
		if (!charController) {
			return {};
		}

		return SelectTargetsInFront(pred, numberOfPrey, THIGH_ANGLE, false, [pred, this](auto prey) {
			return this->CanThighCrush(pred, prey);
		});
	}

	bool ThighCrushController::CanThighCrush(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}

		if (prey->IsDead()) {
			return false;
		}

		if (AnimationVars::Crawl::IsCrawling(pred) || AnimationVars::General::IsTransitioning(pred) || IsBeingHeld(pred, prey)) {
			return false;
		}

		if (pred->AsActorState()->GetSitSleepState() == SIT_SLEEP_STATE::kIsSitting) { // disallow doing it when using furniture
			return false;	
		}

		float pred_scale = get_visual_scale(pred);
		// No need to check for BB scale in this case

		float sizedifference = get_scale_difference(pred, prey, SizeType::VisualScale, false, true);
		
		float MINIMUM_DISTANCE = MINIMUM_THIGH_DISTANCE + HighHeelManager::GetBaseHHOffset(pred).Length();
		float MINIMUM_THIGHCRUSH_SCALE = Action_AI_ThighCrush;

		if (pred->IsPlayerRef()) {
			MINIMUM_THIGHCRUSH_SCALE = 1.5f; // Used to freeze tinies, Player Only
		}

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale)) {
			if (sizedifference > MINIMUM_THIGHCRUSH_SCALE) {
				if ((!prey->IsPlayerRef() && !CanPerformActionOn(pred, prey, false))) {
					return false;
				}
				return true;
			} else {
				return false;
			}
			return false;
		}
		return false;
	}
}
