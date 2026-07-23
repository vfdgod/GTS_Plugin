#include "Managers/Animation/Controllers/GrabAnimationController.hpp"

#include "Managers/Animation/Utils/AnimationUtils.hpp"

#include "Managers/Animation/AnimationManager.hpp"
#include "Managers/Animation/Grab.hpp"

#include "Managers/GTSSizeManager.hpp"

using namespace GTS;

namespace {

	constexpr float MINIMUM_GRAB_DISTANCE = 85.0f;
	constexpr float GRAB_ANGLE = 70;
	constexpr float PI = std::numbers::pi_v<float>;

	void CantGrabPlayerMessage(Actor* giant, Actor* tiny, float sizedifference) {
		if (sizedifference < Action_Grab) {
			std::string message = std::format("Player is too big to be grabbed x{:.2f}/{:.2f}", sizedifference, Action_Grab);
			NotifyWithSound(tiny, message);
		}
	}

	void DelayedGrabTask(Actor* pred, Actor* prey) { // Needed to fix tinies becoming immune to size stuff if animation wasnt started
		Grab::GrabActor(pred, prey);

		std::string taskname = std::format("GrabCheck_{}_{}", pred->formID, prey->formID);
		ActorHandle giantHandle = pred->CreateRefHandle();
		ActorHandle tinyHandle = prey->CreateRefHandle();

		TaskManager::RunOnce(taskname, [=](auto& update){
			if (!giantHandle) {
				return;
			}
			if (!tinyHandle) {
				return;
			}

			auto giant = giantHandle.get().get();
			auto tiny = tinyHandle.get().get();
			if (!giant || !tiny) {
				return;
			}
			if (!AnimationVars::General::IsGTSBusy(giant)) { // Means anim isn't applied so we cancel everything
				Grab::CancelGrab(giant, tiny);
			}
		});
	}
}

namespace GTS {

	std::vector<Actor*> GrabAnimationController::GetGrabTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (AnimationVars::General::IsGTSBusy(pred)) {
			return {};
		}
		if (!pred) {
			return {};
		}
		auto charController = pred->GetCharController();
		if (!charController) {
			return {};
		}

		auto preys = SelectTargetsInFront(pred, numberOfPrey, GRAB_ANGLE, numberOfPrey == 1 && NeedsFullActionTargetOrdering(pred), [pred, this](auto prey) {
			return this->CanGrab(pred, prey);
		});

		if (numberOfPrey == 1) {
			return GetMaxActionableTinyCount(pred, preys);
		}

		return preys;
	}

	bool GrabAnimationController::CanGrab(Actor* pred, Actor* prey) const {
		if (pred == prey) {
			return false;
		}
		if (prey->IsDead()) {
			return false;
		}
		if (GetAV(prey, ActorValue::kHealth) < 0) {
			return false;
		}
	
		float pred_scale = get_visual_scale(pred);

		float sizedifference = get_scale_difference(pred, prey, SizeType::VisualScale, true, false);

		float MINIMUM_GRAB_SCALE = Action_Grab;
		float MINIMUM_DISTANCE = MINIMUM_GRAB_DISTANCE;

		if (TinyCalamityActionBoostActive(pred) || AnimationVars::Crawl::IsCrawling(pred)) {
			MINIMUM_DISTANCE *= 1.6f;
		}


		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (prey_distance <= MINIMUM_DISTANCE * pred_scale && sizedifference < MINIMUM_GRAB_SCALE) {
			if (pred->IsPlayerRef()) {
				std::string message = std::format("{} is too big to be grabbed: x{:.2f}/{:.2f}.", prey->GetDisplayFullName(), sizedifference, MINIMUM_GRAB_SCALE);
				shake_camera(pred, 0.45f, 0.30f);
				NotifyWithSound(pred, message);
			} else if (this->allow_message && prey->IsPlayerRef() && IsTeammate(pred)) {
				CantGrabPlayerMessage(pred, prey, sizedifference);
			}
			return false;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && sizedifference > MINIMUM_GRAB_SCALE) {
			if (IsFlying(prey)) {
				return false; // Disallow to grab flying dragons
			}
			if ((!prey->IsPlayerRef() && !CanPerformActionOn(pred, prey, false))) {
				return false;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}

	void GrabAnimationController::StartGrab(Actor* pred, Actor* prey) {
		auto& grabbing = GrabAnimationController::GetSingleton();
		if (!grabbing.CanGrab(pred, prey)) {
			return;
		}

		float shrinkrate = 0.18f;

		if (pred->IsSneaking()) {
			shrinkrate = 0.13f;
		}

		if (TinyCalamity_ShouldShrinkFirst(pred, prey, Action_Grab, 10.2f, 0.18f, 0.13f)) {
			return;
		}
	
		DelayedGrabTask(pred, prey);
		Utils_UpdateHighHeelBlend(pred, false);
		AnimationManager::StartAnim("GrabSomeone", pred);
	}

	void GrabAnimationController::AllowMessage(bool allow) {
		this->allow_message = allow;
	}
}
