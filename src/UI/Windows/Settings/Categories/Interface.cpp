#include "UI/Windows/Settings/Categories/Interface.hpp"

#include "Config/Config.hpp"

#include "UI/Windows/Settings/SettingsWindow.hpp"

#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/Slider.hpp"

#include "UI/Core/ImStyleManager.hpp"
#include "UI/Core/ImWindowManager.hpp"
#include "UI/Core/ImUtil.hpp"

#include "UI/GTSMenu.hpp"


namespace GTS {

	CategoryInterface::CategoryInterface() {
		m_name = "界面";
	}

	void CategoryInterface::DrawLeft(){

		const auto Window = dynamic_cast<SettingsWindow*>(GTSMenu::WindowManager->wSettings);
		if (!Window) return;
	
		WindowSettingsBase_t* Settings = &Window->GetBaseSettings();
		if (!Settings) return;

		//------------  Config Window

		ImUtil_Unique
		{

			PSString T0 = "自动处理此窗口的位置。\n"
						  "禁用后可手动移动并调整设置窗口大小。\n"
						  "如果保持启用，则可在下方调整位置和窗口缩放。";

			PSString T1 = "按屏幕百分比调整窗口大小。";
			PSString T2 = "选择窗口在屏幕上的对齐位置。";
			PSString T3 = "调整相对于所选锚点的偏移。\n"
						  "左/右 | 上/下";

			PSString T5 = "调整设置窗口的不透明度。";
			PSString T6 = "调整设置窗口背景的不透明度。";

			if (ImGui::CollapsingHeader("配置窗口", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::CheckBox("锁定配置窗口位置", &Settings->bLock, T0);
				ImGui::BeginDisabled(!Settings->bLock);

				{
					ImGuiEx::SliderF("窗口大小", &Settings->fWindowSizePercent, 60.0f, 100.0f, T1,"%.0f%%");
					ImGuiEx::ComboEx<ImWindow::WindowAnchor>("锚点", Settings->sAnchor, T2);
					ImGui::BeginDisabled(Settings->sAnchor == "kCenter");

					ImGuiEx::SliderF2("锚点偏移", &Settings->f2Position.at(0), 0.0f, 1280.f, T3, "%.1f");


					ImGui::EndDisabled();
				}

				ImGui::EndDisabled();

				ImGui::Spacing();

				ImGuiEx::SliderF("窗口透明度", &Settings->fAlpha, 0.2f, 1.0f, T5, "%.2fx");
				ImGuiEx::SliderF("背景透明度", &Settings->fBGAlphaMult, 0.2f, 1.0f, T6, "%.2fx");

				ImGui::Spacing();
			}
		}
	}

	void CategoryInterface::DrawRight(){

		// -----  Misc Settings

	    ImUtil_Unique
		{

	        PSString T0 = "选择要显示的计量单位类型。";

	        if(ImGui::CollapsingHeader("杂项设置",ImUtil::HeaderFlagsDefaultOpen)){
				ImGuiEx::ComboEx<LDisplayUnit_t>("计量单位",Config::UI.sDisplayUnits, T0);
	            ImGui::Spacing();
	        }
	    }


		// -----  UI Settings

	    ImUtil_Unique
		{

	        if(ImGui::CollapsingHeader("UI 设置",ImUtil::HeaderFlagsDefaultOpen)){

	            PSString T0 = "调整所有元素和字体的缩放。";
	            PSString T1 = "修改 UI 控件宽度。";
	            PSString T2 = "设置 UI 强调色。";
				PSString T3 = "切换打开阻塞菜单（如设置）时是否完全暂停游戏。\n"
							  "强烈建议不要禁用此项。\n"
		              "关闭并重新打开菜单后生效。";

				PSString T4 = "打开阻塞菜单（如设置）时启用 Skyrim 的背景模糊效果。\n"
							  "关闭并重新打开菜单后生效。";

				PSString T5 = "设置菜单打开时，将游戏速度乘以此值。\n"
				              "仅在“暂停游戏”禁用时生效。";

				ImGuiEx::SliderF("UI 缩放", &Config::UI.fScale, 0.5f, 2.0f, T0,"%.1fx");
	            if (ImGui::IsItemDeactivatedAfterEdit()) {
					ImStyleManager::ApplyStyle();
	            }

				ImGuiEx::SliderF("项目宽度", &Config::UI.fItemWidth, 0.4f, 0.7f, T1,"%.2fx");

	            ImGui::ColorEdit3("强调色", Config::UI.f3AccentColor.data(), ImGuiColorEditFlags_DisplayHSV);
	            if (ImGui::IsItemDeactivatedAfterEdit() || (ImGui::IsItemActive() && ImGui::GetIO().MouseDown[0])) {
					ImStyleManager::ApplyStyle();
	            }
	            if (ImGui::IsItemHovered()){
	                ImGui::SetTooltip(T2);
	            }

				ImGui::Spacing();

				ImGuiEx::CheckBox("暂停游戏", &Config::UI.bDoPause, T3);
				ImGui::SameLine();
				ImGuiEx::CheckBox("背景模糊", &Config::UI.bDoBGBlur, T4);
				ImGuiEx::SliderF("游戏时间倍率", &Config::UI.fSGTMMult, 0.05f, 1.0f, T5, "%.2fx", Config::UI.bDoPause);

	        }
	    }

		ImUtil_Unique
		{
			PSString T0 = "启用/禁用主菜单中显示的信息窗口。";

			if (ImGui::CollapsingHeader("杂项" ,ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("显示启动提示窗口", &Config::Persistent.bShowSplashScreen, T0);
			}
		}
	}
}
