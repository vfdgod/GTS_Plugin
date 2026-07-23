#pragma once
#include "Data/Storage/TransientData.hpp"

namespace GTS {

	class Transient final : public EventListener, public CInitSingleton<Transient> {

		public:
		static void EraseUnloadedData();
		// Previously returned addresses remain allocated if the actor entry is reset or purged.
		static TransientActorData* GetActorData(Actor* actor);

		private:
		virtual std::string DebugName() override;
		virtual void Reset() override;
		virtual void ResetActor(Actor* actor) override;
		virtual void OnGameRevert() override;

		static inline std::mutex _Lock;
		static inline std::unordered_map<FormID, std::unique_ptr<TransientActorData>> TempActorDataMap {};
		// Keep one retirement generation alive so in-flight Havok or main-thread work
		// can finish. The next bulk reset or purge reclaims the previous generation.
		static inline std::vector<std::unique_ptr<TransientActorData>> RetiredActorData {};
	};
}
