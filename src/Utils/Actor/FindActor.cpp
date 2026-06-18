#include <utility>

#include "Utils/Actor/FindActor.hpp"

namespace GTS {

	/**
	 * Find actors using the AIManager process lists.
	 */

	namespace {
		using ActorPtr = decltype(std::declval<ActorHandle>().get());

		struct ActorCache {
			std::uint64_t frame = std::numeric_limits<std::uint64_t>::max();
			std::vector<ActorPtr> actorRefs;
			std::vector<Actor*> actors;
			std::vector<Actor*> teammates;
			std::vector<Actor*> femaleTeammates;
		};

		ActorCache& GetActorCache() {
			thread_local ActorCache cache;
			const auto currentFrame = Time::FramesElapsed();
			if (cache.frame == currentFrame) {
				return cache;
			}

			cache.frame = currentFrame;
			cache.actors.clear();
			cache.teammates.clear();
			cache.femaleTeammates.clear();
			cache.actorRefs.clear();

			const ProcessLists* const process_list = ProcessLists::GetSingleton();
			if (!process_list) {
				return cache;
			}

			Actor* const player = PlayerCharacter::GetSingleton();
			bool playerAdded = false;
			const auto& handles = process_list->highActorHandles;
			cache.actorRefs.reserve(handles.size() + 1);
			cache.actors.reserve(handles.size() + 1);

			for (const auto& handle : handles) {
				if (!handle) {
					continue;
				}

				auto actorPtr = handle.get();
				auto* actor = actorPtr.get();
				if (!actor || !actor->Get3D1(false)) {
					continue;
				}
				if (actor == player) {
					playerAdded = true;
				}

				cache.actorRefs.emplace_back(actorPtr);
				cache.actors.emplace_back(actor);
				if (IsTeammate(actor)) {
					cache.teammates.emplace_back(actor);
					if (IsFemale(actor)) {
						cache.femaleTeammates.emplace_back(actor);
					}
				}
			}

			if (player && player->Get3D1(false) && !playerAdded) {
				cache.actors.emplace_back(player);
			}

			return cache;
		}
	}

	const std::vector<Actor*>& find_actors() {

		GTS_PROFILE_SCOPE("FindActor: FindActors");

		return GetActorCache().actors;
	}


	// This will find up to howMany actors in the scene
	// (not including player and teammate which are ALWAYS returned
	// regardless of howMany are asked for)
	//
	// Any actors not found in this call will instead be returned on next call
	// This means that in frame 1 you can get 10 actors + player team
	// In frame 2 you will get 10 DIFFERENT actors + player team
	// Until all actors have been returned after which you will get previous actors again
	std::vector<Actor*> FindSomeActors(std::string_view tag, uint32_t howMany) {

		static absl::flat_hash_map<std::string, FindActorData> allData;
		auto [it, inserted] = allData.try_emplace(std::string(tag));
		auto& data = it->second;

		//log::info("Looking for actor for {} up to a count of {}", tag, howMany);
		const auto& actors = find_actors();
		std::vector<Actor*> finalActors;
		finalActors.reserve(std::min<std::size_t>(actors.size(), static_cast<std::size_t>(howMany) + 2));
		std::vector<Actor*> notAddedActors;
		notAddedActors.reserve(actors.size());
		uint32_t addedCount = 0;

		for (const auto actor : actors) {
			if (!actor) {
				continue;
			}
			// Player or teammate are always updated
			if (actor->IsPlayerRef() || IsTeammate(actor)) {
				finalActors.push_back(actor);
				//log::info(" - Adding: {}", actor->GetDisplayFullName());
			}
			else if (!data.previousActors.contains(actor->formID) && (addedCount < howMany)) {
				// Other actors are only added if they are not in the previous actor list

				//log::info(" - Adding: {}", actor->GetDisplayFullName());
				finalActors.push_back(actor);
				data.previousActors.insert(actor->formID);
				addedCount += 1;

			}
			else {
				notAddedActors.push_back(actor);
			}
		}
		// Reached the end of all actor
		if (addedCount < howMany) {
			// We need more. Reset the used list and add from
			// those not added set
			data.previousActors.clear();
			for (auto actor : notAddedActors) {
				if (!actor) {
					continue;
				}
				if (addedCount < howMany) {
					finalActors.push_back(actor);
					data.previousActors.insert(actor->formID);
					addedCount += 1;
				} else {
					break;
				}
			}
		}
		return finalActors;
	}

	// Find player teammates
	// But not the player themselves
	const std::vector<Actor*>& FindTeammates() {
		return GetActorCache().teammates;
	}

	const std::vector<Actor*>& FindFemaleTeammates() {
		return GetActorCache().femaleTeammates;
	}
}
