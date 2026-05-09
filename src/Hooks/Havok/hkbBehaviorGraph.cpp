#include "Hooks/Havok/hkbBehaviorGraph.hpp"
#include "Managers/Animation/AnimationManager.hpp"
#include "Hooks/Util/HookUtil.hpp"
#include <limits>
#include <unordered_map>

using namespace GTS;

namespace {

	struct BehaviorGraphCache {
		std::uint64_t frame = std::numeric_limits<std::uint64_t>::max();
		std::unordered_map<hkbBehaviorGraph*, Actor*> actorByBehaviorGraph;
	};

	BehaviorGraphCache& GetBehaviorGraphCache() {
		thread_local BehaviorGraphCache cache;
		const auto currentFrame = Time::FramesElapsed();
		if (cache.frame == currentFrame) {
			return cache;
		}

		cache.frame = currentFrame;
		cache.actorByBehaviorGraph.clear();

		for (auto actor : find_actors()) {
			if (!actor) {
				continue;
			}

			BSAnimationGraphManagerPtr animGraphManager;
			if (!actor->GetAnimationGraphManager(animGraphManager) || !animGraphManager) {
				continue;
			}

			for (auto& graph : animGraphManager->graphs) {
				if (graph && graph->behaviorGraph) {
					cache.actorByBehaviorGraph.try_emplace(graph->behaviorGraph, actor);
				}
			}
		}

		return cache;
	}

	Actor* GetBehaviorGraphActor(hkbBehaviorGraph* a_behaviorGraph) {
		if (!a_behaviorGraph) {
			return nullptr;
		}

		auto& cache = GetBehaviorGraphCache();
		if (auto it = cache.actorByBehaviorGraph.find(a_behaviorGraph); it != cache.actorByBehaviorGraph.end()) {
			return it->second;
		}
		return nullptr;
	}

	float Animation_GetSpeedCorrection(Actor* actor) { // Fixes Hug animation de-sync by copying Gts anim speed to Tiny
		auto transient = Transient::GetActorData(actor);
		if (transient) {
			if (transient->HugAnimationSpeed < 1.0f) {
				return transient->HugAnimationSpeed;
			}
		} 
		return AnimationManager::GetAnimSpeed(actor);
	}
}

namespace Hooks {

	struct hkbBehaviorGraphUpdate {

		static constexpr size_t funcIndex = 0x05;

		static void thunk(hkbBehaviorGraph* a_this, const hkbContext& a_context, float a_timestep) {


			float anim_speed = 1.0f;

			{
				GTS_PROFILE_ENTRYPOINT("HavokBehavior::hkbBehaviorGraphUpdate");

				if (Actor* actor = GetBehaviorGraphActor(a_this)) {
					float multi = Animation_GetSpeedCorrection(actor);
					Perk_ApplyAccelerationPerk(actor, anim_speed);
					anim_speed *= multi;
				}
			}

			func(a_this, a_context, a_timestep * anim_speed);

		}

		FUNCTYPE_VFUNC func;

	};

	void Hook_hkbBehaviorGraph::Install() {
		logger::info("Installing hkbBehaviorGraph VTABLE Hooks...");
		stl::write_vfunc<hkbBehaviorGraphUpdate>(VTABLE_hkbBehaviorGraph[0]);
	}

}
