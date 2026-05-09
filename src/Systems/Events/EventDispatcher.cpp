#include "Systems/Events/EventDispatcher.hpp"
#include "Systems/Events/EventListener.hpp"
#include "Systems/Events/EventData.hpp"

namespace GTS {

	void EventDispatcher::AddListener(EventListener* a_listener) {
		if (!a_listener) return;
		std::lock_guard lock(m_lock);
		logger::debug("Adding Listener: {}", a_listener->DebugName());
		AddListenerTo(m_listeners, a_listener);
		if (a_listener->WantsActorUpdate()) {
			AddListenerTo(m_actorUpdateListeners, a_listener);
		}
		if (a_listener->WantsActorAnimEvent()) {
			AddListenerTo(m_actorAnimEventListeners, a_listener);
		}
	}

	void EventDispatcher::AddListenerTo(tbb::concurrent_vector<ListenerEntry>& listeners, EventListener* a_listener) {
		for (auto& entry : listeners) {
			if (entry.ptr.load(std::memory_order_relaxed) == nullptr) {
				entry.ptr.store(a_listener, std::memory_order_release);
				return;
			}
		}

		listeners.push_back(ListenerEntry(a_listener));
	}

	bool EventDispatcher::RemoveListenerFrom(tbb::concurrent_vector<ListenerEntry>& listeners, EventListener* a_listener) {
		bool removed = false;
		for (auto& entry : listeners) {
			EventListener* current = entry.ptr.load(std::memory_order_relaxed);
			if (current == a_listener) {
				entry.ptr.store(nullptr, std::memory_order_release);
				removed = true;
			}
		}
		return removed;
	}

	void EventDispatcher::CompactListeners(tbb::concurrent_vector<ListenerEntry>& listeners) {
		tbb::concurrent_vector<ListenerEntry> compacted;
		compacted.reserve(listeners.size());

		for (auto& entry : listeners) {
			if (EventListener* listener = entry.ptr.load(std::memory_order_relaxed)) {
				compacted.push_back(ListenerEntry(listener));
			}
		}

		listeners.swap(compacted);
	}

	void EventDispatcher::CompactUnlocked() {
		CompactListeners(m_listeners);
		CompactListeners(m_actorUpdateListeners);
		CompactListeners(m_actorAnimEventListeners);
	}

	void EventDispatcher::RemoveListener(EventListener* a_listener) {
		if (!a_listener) return;
		std::lock_guard lock(m_lock);
		bool removed = false;
		removed |= RemoveListenerFrom(m_listeners, a_listener);
		removed |= RemoveListenerFrom(m_actorUpdateListeners, a_listener);
		removed |= RemoveListenerFrom(m_actorAnimEventListeners, a_listener);

		if (removed) {
			logger::debug("Deleting Listener: {}", a_listener->DebugName());
		}
	}

	void EventDispatcher::Compact() {
		std::lock_guard lock(m_lock);
		CompactUnlocked();
	}

	void EventDispatcher::DoUpdate() {

		////Experiment
		//ForEachListenerConcurrent([](EventListener* listener) {
		//	GTS_PROFILE_SCOPE(listener->DebugName());
		//	listener->Update();
		//});

		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->Update();
		});
	}

	void EventDispatcher::DoActorUpdate(RE::Actor* actor) {

		////Experiment
		//ForEachListenerConcurrent([](EventListener* listener) {
		//	GTS_PROFILE_SCOPE(listener->DebugName());
		//	listener->Update();
		//});

		ForEachActorUpdateListener([actor](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->ActorUpdate(actor);
		});
	}

	void EventDispatcher::DoPapyrusUpdate() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->PapyrusUpdate();
		});
	}

	void EventDispatcher::DoHavokUpdate() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->HavokUpdate();
		});
	}

	void EventDispatcher::DoCameraUpdate() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->CameraUpdate();
		});
	}

	void EventDispatcher::DoReset() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->Reset();
		});
	}

	void EventDispatcher::DoEnabled() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->Enabled();
		});
	}

	void EventDispatcher::DoDisabled() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->Disabled();
		});
	}

	void EventDispatcher::DoStart() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->Start();
		});
	}

	void EventDispatcher::DoDataReady() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->DataReady();
		});
	}


	void EventDispatcher::DoResetActor(RE::Actor* actor) {
		ForEachListener([actor](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->ResetActor(actor);
		});
	}

	void EventDispatcher::DoActorEquip(RE::Actor* actor) {
		ForEachListener([actor](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->ActorEquip(actor);
		});
	}

	void EventDispatcher::DoDragonSoulAbsorption() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->DragonSoulAbsorption();
		});
	}

	void EventDispatcher::DoActorLoaded(RE::Actor* actor) {
		ForEachListener([actor](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->ActorLoaded(actor);
		});
	}

	void EventDispatcher::DoHitEvent(const RE::TESHitEvent* evt) {
		ForEachListener([evt](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->HitEvent(evt);
		});
	}

	void EventDispatcher::DoUnderFootEvent(const UnderFoot& evt) {
		ForEachListener([&evt](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->UnderFootEvent(evt);
		});
	}

	void EventDispatcher::DoOnImpact(const Impact& impact) {
		ForEachListener([&impact](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnImpact(impact);
		});
	}

	void EventDispatcher::DoHighheelEquip(const HighheelEquip& evt) {
		ForEachListener([evt](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnHighheelEquip(evt);
		});
	}

	void EventDispatcher::DoAddPerk(const AddPerkEvent& evt) {
		ForEachListener([evt](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnAddPerk(evt);
		});
	}

	void EventDispatcher::DoRemovePerk(const RemovePerkEvent& evt) {
		ForEachListener([evt](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnRemovePerk(evt);
		});
	}

	void EventDispatcher::DoMenuChange(const RE::MenuOpenCloseEvent* menu_event) {
		ForEachListener([menu_event](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName()); listener->MenuChange(menu_event);
		});
	}

	void EventDispatcher::DoActorAnimEvent(RE::Actor* actor, const RE::BSFixedString& a_tag, const RE::BSFixedString& a_payload) {
		const char* rawTag = a_tag.c_str();
		const char* rawPayload = a_payload.c_str();
		const std::string_view tag = rawTag ? std::string_view(rawTag) : std::string_view();
		const std::string_view payload = rawPayload ? std::string_view(rawPayload) : std::string_view();
		ForEachActorAnimEventListener([actor, tag, payload](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->ActorAnimEvent(actor, tag, payload);
		});
	}

	void EventDispatcher::DoSerdePreSaveEvent() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnGameSave();
		});
	}

	void EventDispatcher::DoSerdePostLoadEvent() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnGameLoaded();
		});
	}

	void EventDispatcher::DoSerdeRevert() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnGameRevert();
		});
	}

	void EventDispatcher::DoConfigResetEvent() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnConfigReset();
		});
	}

	void EventDispatcher::DoConfigRefreshEvent() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnConfigRefresh();
		});
	}

	void EventDispatcher::DoPluginPostLoad() {
		ForEachListener([](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnPluginPostLoad();
		});
	}

	void EventDispatcher::DoFurnitureEvent(const TESFurnitureEvent* a_event) {
		Actor* actor = skyrim_cast<Actor*>(a_event->actor.get());
		TESObjectREFR* object = a_event->targetFurniture.get();
		const bool entering = a_event->type == TESFurnitureEvent::FurnitureEventType::kEnter;
		//logger::trace("Furniture Event Seen");
		//logger::trace("Actor: {}", static_cast<bool>(actor != nullptr));
		//logger::trace("Object: {}", static_cast<bool>(object != nullptr));
		if (actor && object) {
			//logger::trace("Both are true");
			ForEachListener([actor, object, entering](EventListener* listener) {
				GTS_PROFILE_SCOPE(listener->DebugName());
				listener->FurnitureEvent(actor, object, entering);
			});
		}
	}

	void EventDispatcher::DoDeathEvent(const TESDeathEvent* a_event) {
		Actor* killer = skyrim_cast<Actor*>(a_event->actorKiller.get());
		Actor* victim = skyrim_cast<Actor*>(a_event->actorDying.get());
		const bool dead = a_event->dead;

		ForEachListener([killer, victim, dead](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->DeathEvent(killer, victim, dead);
		});
	}

	void EventDispatcher::DoGTSLevelUpEvent(RE::Actor* a_actor) {
		ForEachListener([a_actor](EventListener* listener) {
			GTS_PROFILE_SCOPE(listener->DebugName());
			listener->OnGTSLevelUp(a_actor);
		});
	}
}
