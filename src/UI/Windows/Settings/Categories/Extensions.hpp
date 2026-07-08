#pragma once

#include "Config/Settings/SettingsKeybinds.hpp"
#include "UI/Core/ImCategorySplit.hpp"

namespace GTS {

	class CategoryExtensions final : public ImCategorySplit {
		public:
		CategoryExtensions();
		void DrawLeft() override;
		void DrawRight() override;

		private:
		void DrawExtensionKeybinds();
		bool DrawExtensionInputEvent(BaseEventData_t& a_event, const InputEvent_t& a_defaultEvent);
		void SetWindowBusy(bool a_busy);

		std::string m_rebindingEvent = {};
		std::vector<std::string> m_tempKeys = {};
	};

}
