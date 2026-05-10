#include "UI/Windows/Other/SplashWindow.hpp"
#include "UI/Core/ImFontManager.hpp"
#include "Config/Util/KeybindHandler.hpp"
#include "Config/Keybinds.hpp"

#include "Version.hpp"

#include "Config/Config.hpp"

namespace {

	//Helper function to get the current keybind for opening the settings menu
	std::string FormatKeyList(const std::vector<std::string>& keys) {
		if (keys.empty()) {
			return "未绑定";
		}
		return std::accumulate(std::next(keys.begin()), keys.end(), keys.front(), [](const std::string& a, const std::string& b) {
			return a + " + " + b;
		});
	}

	template <typename Events>
	std::string FindSettingsKeybind(const Events& events) {
		for (const auto& evt : events) {
			if (evt.Event == "OpenModSettings") {
				return FormatKeyList(evt.Keys);
			}
		}
		return {};
	}

	std::string GetSettingsKeybind() {
		if (auto keybind = FindSettingsKeybind(GTS::Keybinds::InputEvents); !keybind.empty()) {
			return keybind;
		}

		if (auto keybind = FindSettingsKeybind(GTS::KeybindHandler::GetAllInputEvents()); !keybind.empty()) {
			return keybind;
		}
		return "未绑定";
	}

}

namespace GTS {

	SplashWindow::~SplashWindow() {
		logger::warn("Running SplashWindow dtor");
		EventDispatcher::RemoveListener(this);
	}

	void SplashWindow::Draw() {

		ImGui::SetWindowPos(GetAnchorPos(m_anchorPos, {20.0f, 15.0f}, false));

		{
			ImFontManager::Push(ImFontManager::kWidgetBody, 1.2f);

			ImGui::Text(
				"Size Matters - %s\n"
				"在游戏内按 %s，或在控制台输入 \"gts menu\" 打开模组设置。",
				GTSPlugin::ModVersion.string(".").c_str(), 
				GetSettingsKeybind().c_str()
			);


			ImFontManager::Pop();
		}

	}

	bool SplashWindow::WantsToDraw() {
		return m_isInMainMenu && Config::Persistent.bShowSplashScreen;
	}

	void SplashWindow::Init() {

		EventDispatcher::AddListener(this);

		m_windowType = kWidget;

		m_flags = ImGuiWindowFlags_NoDecoration       |
				  ImGuiWindowFlags_NoSavedSettings    |
				  ImGuiWindowFlags_NoFocusOnAppearing |
				  ImGuiWindowFlags_NoMove             |
				  ImGuiWindowFlags_NoNav;

		m_name = "SplashWindow";
		m_title = "启动信息";
		m_anchorPos = WindowAnchor::kTopLeft;
		m_fadeSettings.visibilityDuration = 10.0f;
		m_fadeSettings.fadeDuration = 3.0f;
		m_fadeSettings.enabled = true;

	}

	float SplashWindow::GetFullAlpha() {
		return 1.0f;
	}

	float SplashWindow::GetBackgroundAlpha() {
		return 0.7f;
	}

	std::string SplashWindow::GetWindowName() {
		return m_name;
	}

	void SplashWindow::RequestClose() {
		//Do nothing
	}

	std::string SplashWindow::DebugName() {
		return std::string("::") + m_name;
	}

	void SplashWindow::MenuChange(const RE::MenuOpenCloseEvent* a_event) {

		//If main menu is opened
		if (a_event && a_event->menuName == RE::MainMenu::MENU_NAME) {
			ResetFadeState();
			m_isInMainMenu = a_event->opening;
		}
	}
}
