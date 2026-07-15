#include "Managers/Animation/StompAssist.hpp"

#include "Config/Config.hpp"
#include "Data/Transient.hpp"
#include "Managers/Animation/Utils/AttachPoint.hpp"
#include "Managers/Animation/Utils/AnimationUtils.hpp"
#include "Scale/ScaleUtils.hpp"
#include "Utils/Actions/ActionUtils.hpp"
#include "Utils/Actor/ActorBools.hpp"
#include "Utils/Actor/AV.hpp"
#include "Utils/Actor/FindActor.hpp"
#include "Utils/Animation/AnimationVars.hpp"

using namespace GTS;

namespace {
	std::mutex StompAssistLock;
	std::unordered_map<FormID, double> StompAssistUntil;

	struct StompAssistTarget {
		ActorHandle handle;
		float distance = 0.0f;
	};

	bool IsAssisted(FormID formID) {
		std::scoped_lock lock(StompAssistLock);
		const double now = Time::WorldTimeElapsed();
		auto it = StompAssistUntil.find(formID);
		if (it == StompAssistUntil.end()) {
			return false;
		}
		if (it->second <= now) {
			StompAssistUntil.erase(it);
			return false;
		}
		return true;
	}

	double SetAssisted(FormID formID, double duration) {
		const double until = Time::WorldTimeElapsed() + duration;
		std::scoped_lock lock(StompAssistLock);
		StompAssistUntil[formID] = until;
		return until;
	}

	void ClearAssisted(FormID formID, double expectedUntil) {
		std::scoped_lock lock(StompAssistLock);
		auto it = StompAssistUntil.find(formID);
		if (it != StompAssistUntil.end() && it->second == expectedUntil) {
			StompAssistUntil.erase(it);
		}
	}

	NiPoint3 GetAssistPosition(Actor* giant, Actor* tiny, bool right) {
		NiPoint3 coordinates = AttachToUnderFoot(giant, tiny, right);
		if (coordinates != NiPoint3(0.0f, 0.0f, 0.0f)) {
			return coordinates;
		}

		const auto footPoints = GetFootCoordinates(giant, right, false);
		if (footPoints.empty()) {
			return NiPoint3(0.0f, 0.0f, 0.0f);
		}

		coordinates = footPoints.front();
		coordinates.z = CastRayDownwards(tiny).z;
		return coordinates;
	}

	bool IsActionEnabled(StompAssistAction action) {
		switch (action) {
			case StompAssistAction::Normal:
				return Config::Advanced.bStompAssistNormal;
			case StompAssistAction::Strong:
				return Config::Advanced.bStompAssistStrong;
			case StompAssistAction::Trample:
				return Config::Advanced.bStompAssistTrample;
		}
		return false;
	}

	bool IsUnavailableTarget(Actor* giant, Actor* tiny) {
		const auto transient = Transient::GetActorData(tiny);
		return (
			tiny->IsDead() ||
			GetAV(tiny, ActorValue::kHealth) <= 0.0f ||
			IsAssisted(tiny->formID) ||
			IsBeingHeld(giant, tiny) ||
			(transient && transient->AboutToBeEaten) ||
			IsBetweenBreasts(tiny) ||
			AnimationVars::Tiny::IsBeingGrinded(tiny) ||
			AnimationVars::Tiny::IsBeingHugged(tiny) ||
			AnimationVars::Tiny::IsInThighs(tiny) ||
			AnimationVars::Tiny::IsUnderButt(tiny)
		);
	}

	std::vector<StompAssistTarget> CollectTargets(Actor* giant, const NiPoint3& footPosition) {
		std::vector<StompAssistTarget> targets;
		constexpr float SEARCH_RADIUS_MULT = 1.6f;
		const float giantScale = get_visual_scale(giant);
		const float maxDistance = std::max(0.0f, Config::Advanced.fStompAssistSearchRadius) * SEARCH_RADIUS_MULT * giantScale;
		const float maxDistanceSquared = maxDistance * maxDistance;
		const float sizeThreshold = std::max(1.0f, Config::Advanced.fStompAssistSizeThreshold);
		const NiPoint3 giantPosition = giant->GetPosition();

		for (auto tiny : find_actors()) {
			if (!tiny || tiny == giant || !tiny->Is3DLoaded()) {
				continue;
			}

			const NiPoint3 actorPosition = tiny->GetPosition();
			const NiPoint3 actorDelta = actorPosition - giantPosition;
			const float actorDistanceSquared = actorDelta.x * actorDelta.x + actorDelta.y * actorDelta.y + actorDelta.z * actorDelta.z;
			if (actorDistanceSquared > maxDistanceSquared) {
				continue;
			}

			if (get_scale_difference(giant, tiny, SizeType::VisualScale, false, true) < sizeThreshold) {
				continue;
			}

			if (!CanDoDamage(giant, tiny, false) || IsUnavailableTarget(giant, tiny)) {
				continue;
			}

			const NiPoint3 footDelta = actorPosition - footPosition;
			const float footDistanceSquared = footDelta.x * footDelta.x + footDelta.y * footDelta.y + footDelta.z * footDelta.z;
			targets.push_back({
				.handle = tiny->CreateRefHandle(),
				.distance = footDistanceSquared,
			});
		}

		std::sort(targets.begin(), targets.end(), [](const StompAssistTarget& lhs, const StompAssistTarget& rhs) {
			return lhs.distance < rhs.distance;
		});

		return targets;
	}

	bool HoldTargetUnderFoot(Actor* giant, Actor* tiny, bool right, float duration) {
		const NiPoint3 initialPosition = GetAssistPosition(giant, tiny, right);
		if (initialPosition == NiPoint3(0.0f, 0.0f, 0.0f)) {
			return false;
		}

		ActorHandle giantHandle = giant->CreateRefHandle();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		const FormID tinyFormID = tiny->formID;
		const double startTime = Time::WorldTimeElapsed();
		const std::string taskName = std::format("StompAssist_{}_{}", giant->formID, tinyFormID);

		TaskManager::Cancel(taskName);
		const double assistUntil = SetAssisted(tinyFormID, duration);
		AttachTo(giant, tiny, initialPosition);
		TaskManager::RunFor(taskName, duration, [=](auto& update) {
			if (!giantHandle || !tinyHandle) {
				ClearAssisted(tinyFormID, assistUntil);
				return false;
			}

			Actor* giantRef = giantHandle.get().get();
			Actor* tinyRef = tinyHandle.get().get();
			if (!giantRef || !tinyRef || tinyRef->IsDead()) {
				ClearAssisted(tinyFormID, assistUntil);
				return false;
			}

			if (Time::WorldTimeElapsed() - startTime > duration || !CanDoDamage(giantRef, tinyRef, false)) {
				ClearAssisted(tinyFormID, assistUntil);
				return false;
			}

			const NiPoint3 coordinates = GetAssistPosition(giantRef, tinyRef, right);
			if (coordinates == NiPoint3(0.0f, 0.0f, 0.0f)) {
				ClearAssisted(tinyFormID, assistUntil);
				return false;
			}

			AttachTo(giantRef, tinyRef, coordinates);
			if (update.progress >= 1.0) {
				ClearAssisted(tinyFormID, assistUntil);
				return false;
			}
			return true;
		});

		TaskManager::ChangeUpdate(taskName, UpdateKind::Havok);
		return true;
	}
}

namespace GTS {
	bool IsStompAssistActive(Actor* actor) {
		return actor && IsAssisted(actor->formID);
	}

	float GetStompAssistSearchDistance(Actor* giant) {
		if (!giant) {
			return 0.0f;
		}
		constexpr float SEARCH_RADIUS_MULT = 1.6f;
		return std::max(0.0f, Config::Advanced.fStompAssistSearchRadius) * SEARCH_RADIUS_MULT * get_visual_scale(giant);
	}

	float GetStompAssistSizeThreshold() {
		return std::max(1.0f, Config::Advanced.fStompAssistSizeThreshold);
	}

	void TryStompAssist(Actor* giant, bool right, StompAssistAction action, Actor* preferredTarget) {
		if (!giant || !IsActionEnabled(action)) {
			return;
		}

		// Player: needs the global assist toggle.
		// AI/teammate: allow when AI stomp fix is on, so AI can lock the selected prey underfoot.
		const bool allowPlayer = giant->IsPlayerRef() && Config::Advanced.bStompAssist;
		const bool allowAI = !giant->IsPlayerRef() && Config::AI.bStompKickAutoAimFix && preferredTarget;
		if (!allowPlayer && !allowAI) {
			return;
		}

		const auto footPoints = GetFootCoordinates(giant, right, false);
		if (footPoints.empty()) {
			return;
		}

		const float duration = std::clamp(Config::Advanced.fStompAssistDuration, 0.2f, 2.0f);

		// AI path: only assist the already-selected prey.
		if (preferredTarget) {
			const auto transient = Transient::GetActorData(preferredTarget);
			const bool blockedState =
				IsAssisted(preferredTarget->formID) ||
				IsBeingHeld(giant, preferredTarget) ||
				(transient && transient->AboutToBeEaten) ||
				IsBetweenBreasts(preferredTarget) ||
				AnimationVars::Tiny::IsBeingGrinded(preferredTarget) ||
				AnimationVars::Tiny::IsBeingHugged(preferredTarget) ||
				AnimationVars::Tiny::IsInThighs(preferredTarget) ||
				AnimationVars::Tiny::IsUnderButt(preferredTarget);
			// AI may still assist crushable corpses selected by stomp AI; skip other locked states.
			if (blockedState) {
				return;
			}

			HoldTargetUnderFoot(giant, preferredTarget, right, duration);
			return;
		}

		auto targets = CollectTargets(giant, footPoints.front());
		if (targets.empty()) {
			return;
		}

		const std::size_t maxTargets = Config::Advanced.bStompAssistMultiTarget
			? std::clamp<std::size_t>(Config::Advanced.iStompAssistMaxTargets, 1, 8)
			: 1;

		std::size_t attached = 0;
		for (const auto& target : targets) {
			if (attached >= maxTargets) {
				break;
			}

			Actor* tiny = target.handle.get().get();
			if (!tiny) {
				continue;
			}

			if (HoldTargetUnderFoot(giant, tiny, right, duration)) {
				attached += 1;
			}
		}
	}
}
