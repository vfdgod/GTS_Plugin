#include "Data/Transient.hpp"

namespace GTS {

	std::string Transient::DebugName() {
		return "::Transient";
	}

	void Transient::Reset() {
		std::unique_lock lock(_Lock);
		RetiredActorData.clear();
		constexpr auto noAIAcquireFlag = static_cast<std::uint32_t>(Actor::RecordFlags::kNoAIAcquire);
		for (const auto& [formID, data] : TempActorDataMap) {
			if (data && data->FollowerNoAIAcquireAdded) {
				if (auto actor = TESForm::LookupByID<Actor>(formID)) {
					actor->formFlags &= ~noAIAcquireFlag;
				}
			}
		}
		for (auto& [formID, data] : TempActorDataMap) {
			RetiredActorData.emplace_back(std::move(data));
		}
		TempActorDataMap.clear();
		logger::info("Transient was reset");
	}

	void Transient::ResetActor(Actor* actor) {
		std::unique_lock lock(_Lock);
		if (actor) {
			auto key = actor->formID;
			if (auto it = TempActorDataMap.find(key); it != TempActorDataMap.end()) {
				if (it->second && it->second->FollowerNoAIAcquireAdded) {
					constexpr auto noAIAcquireFlag = static_cast<std::uint32_t>(Actor::RecordFlags::kNoAIAcquire);
					actor->formFlags &= ~noAIAcquireFlag;
				}
				RetiredActorData.emplace_back(std::move(it->second));
				TempActorDataMap.erase(it);
			}
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
			return it->second.get();
		}

		if (get_scale(actor) < 0.0f || !actor->Is3DLoaded()) {
			return nullptr;
		}

		auto data = std::make_unique<TransientActorData>(actor);
		auto* result = data.get();
		TempActorDataMap.emplace(actorKey, std::move(data));
		return result;
	}

	void Transient::EraseUnloadedData() {
		std::unique_lock lock(_Lock);
		RetiredActorData.clear();

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

		constexpr auto noAIAcquireFlag = static_cast<std::uint32_t>(Actor::RecordFlags::kNoAIAcquire);
		for (auto it = TempActorDataMap.begin(); it != TempActorDataMap.end();) {
			const bool shouldErase = !allowedFormIDs.contains(it->first);
			if (shouldErase) {
				if (it->second && it->second->FollowerNoAIAcquireAdded) {
					if (auto actor = TESForm::LookupByID<Actor>(it->first)) {
						actor->formFlags &= ~noAIAcquireFlag;
					}
				}
				RetiredActorData.emplace_back(std::move(it->second));
				it = TempActorDataMap.erase(it);
			}
			else {
				++it;
			}
		}

		logger::critical("All Unloaded actors have beeen purged from transient.");
	}
}
