#pragma once

#include "Systems/Events/EventData.hpp"
#include <utility>

namespace GTS {

	class EventListener;

	class EventDispatcher {

		public:

		static void AddListener(EventListener* a_listener);
		static void RemoveListener(EventListener* a_listener);

		static void DoUpdate();
		static void DoActorUpdate(RE::Actor* actor);
		static void DoPapyrusUpdate();
		static void DoHavokUpdate();
		static void DoCameraUpdate();
		static void DoReset();
		static void DoEnabled();
		static void DoDisabled();
		static void DoStart();
		static void DoDataReady();
		static void DoResetActor(RE::Actor* actor);
		static void DoActorEquip(RE::Actor* actor);
		static void DoDragonSoulAbsorption();
		static void DoActorLoaded(RE::Actor* actor);
		static void DoHitEvent(const RE::TESHitEvent* evt);
		static void DoUnderFootEvent(const UnderFoot& evt);
		static void DoOnImpact(const Impact& impact);
		static void DoHighheelEquip(const HighheelEquip& evt);
		static void DoAddPerk(const AddPerkEvent& evt);
		static void DoRemovePerk(const RemovePerkEvent& evt);
		static void DoMenuChange(const RE::MenuOpenCloseEvent* menu_event);
		static void DoActorAnimEvent(RE::Actor* actor, const RE::BSFixedString& a_tag, const RE::BSFixedString& a_payload);
		static void DoSerdePreSaveEvent();
		static void DoSerdePostLoadEvent();
		static void DoSerdeRevert();
		static void DoConfigResetEvent();
		static void DoConfigRefreshEvent();
		static void DoPluginPostLoad();
		static void DoFurnitureEvent(const TESFurnitureEvent* a_event);
		static void DoDeathEvent(const TESDeathEvent* a_event);
		static void DoGTSLevelUpEvent(RE::Actor* a_actor);

	private:

		struct ListenerEntry {
			std::atomic<EventListener*> ptr{ nullptr };
			std::atomic_uint32_t activeCalls{ 0 };

            ListenerEntry() = default;
            explicit ListenerEntry(EventListener* p) : ptr(p) {}

			ListenerEntry(const ListenerEntry& other) : ptr(other.ptr.load(std::memory_order_relaxed)) {}
			ListenerEntry& operator=(const ListenerEntry& other) {
				ptr.store(other.ptr.load(std::memory_order_relaxed), std::memory_order_relaxed);
				activeCalls.store(0, std::memory_order_relaxed);
				return *this;
			}

            ListenerEntry(ListenerEntry&& other) noexcept : ptr(other.ptr.exchange(nullptr, std::memory_order_relaxed)) {}
			ListenerEntry& operator=(ListenerEntry&& other) noexcept {
				ptr.store(other.ptr.exchange(nullptr, std::memory_order_relaxed), std::memory_order_relaxed);
				activeCalls.store(0, std::memory_order_relaxed);
				return *this;
			}

			EventListener* TryAcquire() {
				for (;;) {
					EventListener* listener = ptr.load(std::memory_order_acquire);
					if (!listener) {
						return nullptr;
					}

					activeCalls.fetch_add(1, std::memory_order_acq_rel);
					if (ptr.load(std::memory_order_acquire) == listener) {
						return listener;
					}
					Release();
				}
			}

			void Release() {
				if (activeCalls.fetch_sub(1, std::memory_order_acq_rel) == 1) {
					activeCalls.notify_all();
				}
			}

			bool Deactivate(EventListener* a_listener) {
				EventListener* expected = a_listener;
				if (!ptr.compare_exchange_strong(expected, nullptr, std::memory_order_acq_rel)) {
					return false;
				}

				for (auto active = activeCalls.load(std::memory_order_acquire); active != 0; active = activeCalls.load(std::memory_order_acquire)) {
					activeCalls.wait(active, std::memory_order_acquire);
				}
				return true;
			}
		};

		static inline std::mutex m_lock;
        static inline tbb::concurrent_vector<ListenerEntry> m_listeners;
		static inline tbb::concurrent_vector<ListenerEntry> m_actorUpdateListeners;
		static inline tbb::concurrent_vector<ListenerEntry> m_actorAnimEventListeners;

		static void AddListenerTo(tbb::concurrent_vector<ListenerEntry>& listeners, EventListener* a_listener);
		static bool RemoveListenerFrom(tbb::concurrent_vector<ListenerEntry>& listeners, EventListener* a_listener);

		template <typename Container, typename Func>
		static void ForEachListenerIn(Container& listeners, Func&& func) {
			for (auto& entry : listeners) {
				if (EventListener* listener = entry.TryAcquire()) {
					struct ReleaseGuard {
						ListenerEntry& entry;
						~ReleaseGuard() { entry.Release(); }
					} releaseGuard{ entry };
					func(listener);
				}
			}
        }

		template <typename Func>
		static void ForEachListener(Func&& func) {
			ForEachListenerIn(m_listeners, std::forward<Func>(func));
		}

		template <typename Func>
		static void ForEachActorUpdateListener(Func&& func) {
			ForEachListenerIn(m_actorUpdateListeners, std::forward<Func>(func));
		}

		template <typename Func>
		static void ForEachActorAnimEventListener(Func&& func) {
			ForEachListenerIn(m_actorAnimEventListeners, std::forward<Func>(func));
		}

	};
}
