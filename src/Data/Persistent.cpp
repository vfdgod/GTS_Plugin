#include "Data/Persistent.hpp"

namespace GTS {

	//----------------------
	// Overriden Virtuals
	//----------------------

	std::string Persistent::DebugName() {
		return "::Persistent";
	}

	void Persistent::Reset() {
		{
			std::unique_lock lock(_Lock);
			ClearData();
		}

		// Ensure we reset them back to inital scales
		// if they are loaded into game memory
		// since skyrim only lazy loads actors
		// that are already in memory it won't reload
		// their nif scales otherwise
		for (const auto& actor : find_actors()) {
			ResetToInitScale(actor);
		}
		ClearInitialScales();

		logger::info("Persistent: Reset Event");

	}

	void Persistent::ResetActor(Actor* actor) {
		if (!actor) {
			return;
		}

		// Fired after a TESReset event
		// This event should be when the game attempts to reset their
		// actor values etc when the cell resets

		auto key = actor->formID;
		bool shouldResetScale = false;

		{
			std::unique_lock lock(_Lock);
			if (this->ActorMap.Reset(key)) {
				shouldResetScale = true;
			}
		}

		if (shouldResetScale) {
			ResetToInitScale(actor);
		}
	}

	//----------------------
	// SKSE Callbacks
	//----------------------

	void Persistent::OnGameLoaded(SKSE::SerializationInterface* serde) {

		logger::debug("Persistent OnGameLoaded Start");

		{
			std::unique_lock lock(_Lock);
			LoadPersistent(serde);
		}
	
		logger::info("Persistent OnGameLoaded OK");
#ifndef GTS_DISABLE_PLUGIN
		EventDispatcher::DoSerdePostLoadEvent();
#endif
	}

	void Persistent::OnGameSaved(SKSE::SerializationInterface* serde) {

#ifndef GTS_DISABLE_PLUGIN
		EventDispatcher::DoSerdePreSaveEvent();
#endif
		logger::debug("Persistent OnGameSaved Start");

		{
			std::unique_lock lock(_Lock);
			SavePersistent(serde);
		}

		logger::info("Persistent OnGameSaved OK");
	}

	void Persistent::OnRevert(SKSE::SerializationInterface*) {
#ifndef GTS_DISABLE_PLUGIN
		logger::info("Persistent::OnRevert");
		GetSingleton().Reset();
		EventDispatcher::DoSerdeRevert();
#endif
	}

	//---------------------------
	// Save/Load Implementations
	//---------------------------

	void Persistent::LoadPersistent(SKSE::SerializationInterface* serde) {
		std::uint32_t RecordType;
		std::uint32_t RecordSize;
		std::uint32_t RecordVersion;

		logger::info("De-Serializing Persistent...");

		// Always start from clean defaults so missing records in older saves
		// do not inherit stale values from the previously loaded save/session.
		ClearData();

		while (serde->GetNextRecordInfo(RecordType, RecordVersion, RecordSize)) {

			//----- Actor Data structs
			ActorMap.Load(serde, RecordType, RecordVersion, RecordSize);
			KillCountMap.Load(serde, RecordType, RecordVersion, RecordSize);

			//----- Camera
			TrackedCameraState.Load(serde, RecordType, RecordVersion, RecordSize);

			//----- Crawk/Sneak State
			EnableCrawlPlayer.Load(serde, RecordType, RecordVersion, RecordSize);
			EnableCrawlFollower.Load(serde, RecordType, RecordVersion, RecordSize);

			// ---- Quest Progression
			HugStealCount.Load(serde, RecordType, RecordVersion, RecordSize);
			StolenSize.Load(serde, RecordType, RecordVersion, RecordSize);
			CrushCount.Load(serde, RecordType, RecordVersion, RecordSize);
			STNCount.Load(serde, RecordType, RecordVersion, RecordSize);
			HandCrushed.Load(serde, RecordType, RecordVersion, RecordSize);
			VoreCount.Load(serde, RecordType, RecordVersion, RecordSize);
			GiantCount.Load(serde, RecordType, RecordVersion, RecordSize);

			// ---- Ability Info
			MSGSeenTinyCamity.Load(serde, RecordType, RecordVersion, RecordSize);
			MSGSeenGrowthSpurt.Load(serde, RecordType, RecordVersion, RecordSize);
			MSGSeenAspectOfGTS.Load(serde, RecordType, RecordVersion, RecordSize);

			// ---- Mod Settings
			ModSettings.Load(serde, RecordType, RecordVersion, RecordSize);

		}
	}

	void Persistent::SavePersistent(SKSE::SerializationInterface* serde) {

		logger::info("Serializing Persistent...");

		//----- Actor Data Structs
		ActorMap.Save(serde);
		KillCountMap.Save(serde);

		//----- Camera
		TrackedCameraState.Save(serde);

		//----- Crawk/Sneak State
		EnableCrawlPlayer.Save(serde);
		EnableCrawlFollower.Save(serde);

		// ---- Quest Progression
		HugStealCount.Save(serde);
		StolenSize.Save(serde);
		CrushCount.Save(serde);
		STNCount.Save(serde);
		HandCrushed.Save(serde);
		VoreCount.Save(serde);
		GiantCount.Save(serde);

		// ---- Ability Info
		MSGSeenTinyCamity.Save(serde);
		MSGSeenGrowthSpurt.Save(serde);
		MSGSeenAspectOfGTS.Save(serde);

		// ---- Mod Settings
		ModSettings.Save(serde);
	}

	//---------------------------
	// Getters
	//---------------------------

	PersistentActorData* Persistent::GetActorData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		return GetActorData(*actor);
	}

	PersistentActorData* Persistent::GetActorData(Actor& actor) {
		std::unique_lock lock(_Lock);
		auto key = actor.formID;

		// This accessor sits on the per-frame actor update path, so keep the hit
		// path to a single lookup and only allocate on a validated miss.
		if (auto* data = ActorMap.Find(key)) {
			return data;
		}

		if (!actor.Is3DLoaded() || get_scale(&actor) < 0.0f) {
			return nullptr;
		}

		return ActorMap.TryEmplace(key);
	}

	PersistentKillCountData* Persistent::GetKillCountData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		return GetKillCountData(*actor);
	}

	PersistentKillCountData* Persistent::GetKillCountData(Actor& actor) {
		std::unique_lock lock(_Lock);
		auto key = actor.formID;
		if (auto* data = KillCountMap.Find(key)) {
			return data;
		}

		if (!actor.Is3DLoaded()) {
			return nullptr;
		}

		return KillCountMap.TryEmplace(key);
	}

	//---------------------------
	// Data Management
	//---------------------------

	void Persistent::ClearData() {

		ActorMap.Clear();
		KillCountMap.Clear();

		TrackedCameraState       = 0;
		EnableCrawlPlayer        = false;
		EnableCrawlFollower      = false;
		HugStealCount            = 0.0f;
		StolenSize               = 0.0f;
		CrushCount               = 0.0f;
		STNCount                 = 0.0f;
		HandCrushed              = 0.0f;
		VoreCount                = 0.0f;
		GiantCount               = 0.0f;
		MSGSeenTinyCamity        = false;
		MSGSeenGrowthSpurt       = false;
		MSGSeenAspectOfGTS       = false;
		ModSettings.value.clear();

	}

	void Persistent::EraseUnloadedData() {
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

		// Iterate through ActorDataMap and remove entries whose key is not in allowedFormIDs.
		ActorMap.EraseIf([&](FormID formID) {
			return !allowedFormIDs.contains(formID);
		});

		// Iterate through KillCountMap and remove entries whose key is not in allowedFormIDs.
		KillCountMap.EraseIf([&](FormID formID) {
			return !allowedFormIDs.contains(formID);
		});

		logger::critical("All Unloaded actors have beeen purged from persistent.");
	}
}
