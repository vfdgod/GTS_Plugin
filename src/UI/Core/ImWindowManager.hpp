#pragma once

#include "UI/Core/ImContextManager.hpp"
#include "UI/Core/ImWindow.hpp"
#include "UI/Windows/Settings/SettingsWindow.hpp"

namespace GTS {

    class ImWindowManager {

		private:
        std::vector<std::unique_ptr<ImWindow>> Windows;
        std::unique_ptr<ImContextManager> Context;
        float m_lastFrameTime = 0.0f;
        std::vector<Actor*> m_cachedTeamMateList;

		public:
		// Accessors for common windows
        ImWindow* wSplash = nullptr;
        ImWindow* wSettings = nullptr;
        ImWindow* wDebug = nullptr;

        ImWindow* wKillFeed = nullptr;
        ImWindow* wActionMenu = nullptr;
        ImWindow* wQuestWidget = nullptr;

        //Widgets
        ImWindow* wUBar = nullptr;
        ImWindow* wStatusBar = nullptr;

		// Size Bars
        ImWindow* wSBarP = nullptr;
        ImWindow* wSBarF1 = nullptr;
        ImWindow* wSBarF2 = nullptr;
        ImWindow* wSBarF3 = nullptr;
        ImWindow* wSBarF4 = nullptr;
        ImWindow* wSBarF5 = nullptr;

        [[nodiscard]] ImWindow* GetWindowByName(const std::string& a_name) const;
        [[nodiscard]] bool HasInputConsumers();
        [[nodiscard]] bool HasActiveWindows();
        bool CloseInputConsumers() const;
        [[nodiscard]] ImWindow::WindowType GetHighestVisibleWindowType() const;
        void AddWindow(std::unique_ptr<ImWindow> a_window, ImWindow** a_accessor = nullptr);
        void Update();
        void Init();
        int8_t GetDesiredPriority() const;
        bool GetDesiredCursorState() const;
        bool GetWantDrawOnPresent() const;
        [[nodiscard]] const std::vector<RE::Actor*>& GetCachedTeamMateList() const;

    };

}
