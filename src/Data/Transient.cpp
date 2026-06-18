#include "Data/Transient.hpp"

namespace GTS {

	std::string Transient::DebugName() {
		return "::Transient";
	}

	void Transient::Reset() {
		std::unique_lock lock(_Lock);
		TempActorDataMap.clear();
		logger::info("Transient was reset");
	}

	void Transient::ResetActor(Actor* actor) {
		std::unique_lock lock(_Lock);
		if (actor) {
			auto key = actor->formID;
			TempActorDataMap.erase(key);
		}
	}

	void Transient::OnGameRevert() {
		Reset();
	}

	TransientActorData* Transient::GetActorData(Actor* actor) {
		std::unique_lock lock(_Lock);

		if (!actor) {
			return nullptr;
		}
		auto actorKey = actor->formID;

		// This is hit from many frame-sensitive systems; keep the fast hit path
		// to a single lookup and only construct transient data on a validated miss.
		if (auto it = TempActorDataMap.find(actorKey); it != TempActorDataMap.end()) {
			return &(it->second);
		}

		if (get_scale(actor) < 0.0f || !actor->Is3DLoaded()) {
			return nullptr;
		}

		auto [iter, inserted] = TempActorDataMap.try_emplace(actorKey, actor);
		return &(iter->second);
	}

	void Transient::EraseUnloadedData() {
		std::unique_lock lock(_Lock);

		// Create a set to hold the whitelisted FormIDs.
		std::unordered_set<FormID> allowedFormIDs;

		// Always keep FormID 0x14 (Player).
		allowedFormIDs.insert(0x14);

		// Get preserve all currently loaded actors
		for (const Actor* ActorToNotDelete : find_actors()) {
			if (ActorToNotDelete) {
				allowedFormIDs.insert(ActorToNotDelete->formID);
			}
		}

		std::erase_if(TempActorDataMap, [&](const auto& entry) {
			return !allowedFormIDs.contains(entry.first);
		});

		logger::critical("All Unloaded actors have beeen purged from transient.");
	}
}
