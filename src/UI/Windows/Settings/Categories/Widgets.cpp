#include "UI/Windows/Settings/Categories/Widgets.hpp"

#include "Config/Config.hpp"

#include "UI/GTSMenu.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/Misc.hpp"
#include "UI/Controls/ProgressBar.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Core/ImColorUtils.hpp"

#include "UI/Core/ImUtil.hpp"
#include "UI/Windows/Other/KillFeedWindow.hpp"

#include "UI/Windows/Widgets/SizeBarWindow.hpp"
#include "UI/Windows/Widgets/StatusBarWindow.hpp"
#include "UI/Windows/Widgets/USBarWindow.hpp"

namespace {

	// Common Bar Tooltip strings
	PSString TCVis = "切换组件可见性。";
	PSString TCAnchor = "设置锚点。";
	PSString TCPosOffset = "相对于所选锚点偏移组件位置。";
	PSString TCHeight = "调整进度条高度。";
	PSString TCWidth = "调整进度条长度。";
	PSString TCAlpha = "调整整体透明度。";
	PSString TCIFadeEn = "在指定时间无变化后隐藏进度条。";
	PSString TCIFadeTime = "设置无变化后进度条开始淡出的秒数。";
	PSString TCIFadeDelta = "设置进度条隐藏后重新出现所需的最小变化量。";
	PSString TCThickness = "调整进度条边框粗细。";
	PSString TCBorderCol = "调整进度条边框的灰度明暗。";
	PSString TCBorderAlpha = "调整进度条边框透明度。";
	PSString TCGradientEn = "启用颜色渐变（会逐渐加深/变亮进度条填充部分的边缘）。";
	PSString TCDualColor = "为渐变使用不同颜色。";
	PSString TCSwapDir = "反转渐变方向。";
	PSString TCGradientIntens = "调整单色渐变的明暗强度。";
	PSString TCRoundingEn = "启用进度条圆角。";
	PSString TCRoundingAmt = "调整进度条圆角大小。";

	//Specific to USBar
	PSString TShowUStompMult = "显示当前脚下踩踏角度动画偏移倍率。";
	PSString TShowAbsAngle = "以角度显示当前绝对镜头上/下角度。";

	//Specific to SizeBar
	PSString TShowSName = "显示此体型条代表的角色名称。";
	PSString TShowSScale = "显示此体型条代表的角色当前体型倍率。";
	PSString TShowSSize = "显示此体型条代表的角色当前格式化体型（英尺/米）。";
	PSString TCopyPlayer = "复制玩家体型条的样式设置；颜色和位置不会复制。";

	//Specific to Icon/Buff/Status Bar
	PSString TFrameAlpha = "设置包含状态栏窗口的透明度。";
	PSString TIcoVis = "切换图标可见性。";
	PSString TIcoAlwaysVis = "切换图标为空时是否仍保持可见。";
	PSString TIcoSize = "设置图标大小。";
	PSString TRelativeFontScale = "设置相对于图标大小的字体大小。";
	PSString TCopyAccent = "将强调色设为渐变颜色。";


	PSString KFFlagNoKiller = "显示/隐藏击杀者名称。";
	PSString KFFlagNoKillType = "显示/隐藏死亡类型。";
	PSString KFBGAlpha = "设置背景透明度。";
	PSString KFVisDur = "设置击杀条目的显示持续时间。";
	PSString KFWidth = "设置击杀条目宽度。";
	PSString KFMaxVis = "设置同时可见的最大击杀条目数量。\n"
					    "超出的条目会进入队列，并在当前可见条目过期后显示。";

	PSString KFResetFontColor = "重置字体颜色";
	PSString KFEnableVanillaKills = "在击杀提示中显示原版游戏击杀（例如普通战斗中的击杀）。";
	PSString KFEnableWorldKills = "显示没有攻击者的原版游戏死亡（例如坠落伤害、脚本死亡等）。";
	PSString KFFontScale = "设置相对于图标大小的字体大小。";


	void DrawKillFeedWindowBase(GTS::ImWindow* a_KillFeed) {

		if (!a_KillFeed) return;
		auto Win = dynamic_cast<GTS::KillFeedWindow*>(a_KillFeed);
		if (!Win) return;

		auto& BaseSettings = Win->GetBaseSettings();

		ImGui::SeparatorText("通用设置");

		ImGuiEx::CheckBox("启用", &BaseSettings.bVisible, TCVis);

		ImGui::BeginDisabled(!BaseSettings.bVisible);
		{
			// Anchor & Position
			ImGuiEx::ComboEx<GTS::ImWindow::WindowAnchor>("锚点", BaseSettings.sAnchor, TCAnchor);
			ImGuiEx::SliderF("位置 上/下", &BaseSettings.f2Position.at(1), 0.0f, 720.f, TCPosOffset, "%.1f");

			if (BaseSettings.sAnchor != "kCenter") {
				ImGuiEx::SliderF("位置 左/右", &BaseSettings.f2Position.at(0), 0.0f, 1280.f, TCPosOffset, "%.1f");
			}

			ImGuiEx::SliderF("透明度", &BaseSettings.fAlpha, 0.1f, 1.0f, TCAlpha, "%.2fx");
		}
		ImGui::EndDisabled();
	}

	void DrawKillFeedOptions(GTS::ImWindow* a_KillFeed) {

		if (!a_KillFeed) return;
		auto Win = dynamic_cast<GTS::KillFeedWindow*>(a_KillFeed);
		if (!Win) return;

		auto& BaseSettings = Win->GetBaseSettings();
		auto& ExtraSettings = Win->GetExtraSettings<WindowSettingsKillFeed_t>();

		ImGui::SeparatorText("扩展设置");

		ImGui::BeginDisabled(!BaseSettings.bVisible);
		{
			
			ImGuiEx::SliderF("显示持续时间", &BaseSettings.fFadeAfter, 0.1f, 10.f, KFVisDur, "%.2f 秒");
			ImGuiEx::SliderF("条目宽度", &ExtraSettings.fWidth, 75.0f, 600.f, KFWidth, "%.0f");
			ImGuiEx::SliderU8("最大可见条目", &ExtraSettings.iMaxVisibleEntries, 2, 20, KFMaxVis, "%.d 条");
			ImGuiEx::SliderF("字体缩放倍率", &ExtraSettings.fFontScaleMult, 0.2f, 3.0f, KFFontScale, "%.2fx");

			float buttonWidth = 18 + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x * 2;

			{
				static GTS::ImGraphics::ImageTransform T = {
					.recolorEnabled = true,
				};

				T.targetColor = ImUtil::Colors::fRGBToImVec4(GTS::Config::UI.f3AccentColor);
				
				if (ImGuiEx::ImageButtonTransform("##ResetA", ImageList::Generic_Square, T, 18, TCopyAccent)) {
					ExtraSettings.f3BGColor = GTS::Config::UI.f3AccentColor;
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
				ImGui::ColorEdit3("背景颜色", ExtraSettings.f3BGColor.data(), ImGuiColorEditFlags_DisplayHSV);
				ImGuiEx::SliderF("背景透明度", &BaseSettings.fBGAlphaMult, 0.0f, 1.0f, KFBGAlpha, "%.2fx");
			}

			{
				
				if (ImGuiEx::ImageButton("##ResetAtt", ImageList::Generic_Reset, 18, KFResetFontColor)) {
					ExtraSettings.f3AttackerColor = {1.0f, 1.0f, 1.0f };
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
				ImGui::ColorEdit3("攻击者文字颜色", ExtraSettings.f3AttackerColor.data(), ImGuiColorEditFlags_DisplayHSV);
			}

			{

				if (ImGuiEx::ImageButton("##ResetVi", ImageList::Generic_Reset, 18, KFResetFontColor)) {
					ExtraSettings.f3VictimColor = { 1.0f, 1.0f, 1.0f };
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
				ImGui::ColorEdit3("受害者文字颜色", ExtraSettings.f3VictimColor.data(), ImGuiColorEditFlags_DisplayHSV);
			}

			{

				if (ImGuiEx::ImageButton("##ResetDT", ImageList::Generic_Reset, 18, KFResetFontColor)) {
					ExtraSettings.f3DeathTypeColor = { .6f, .6f, .6f };
				}

				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
				ImGui::ColorEdit3("死亡类型文字颜色", ExtraSettings.f3DeathTypeColor.data(), ImGuiColorEditFlags_DisplayHSV);
			}

			{
				bool sNoKillType = !(ExtraSettings.iFlags & ImGuiEx::KillFeedEntryFlag_NoKillType);
				bool sNoKiller = !(ExtraSettings.iFlags & ImGuiEx::KillFeedEntryFlag_NoKiller);

				if (ImGuiEx::CheckBox("显示击杀类型", &sNoKillType, KFFlagNoKillType)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlags, ImGuiEx::KillFeedEntryFlag_NoKillType, !sNoKillType);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("显示攻击者", &sNoKiller, KFFlagNoKiller)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlags, ImGuiEx::KillFeedEntryFlag_NoKiller, !sNoKiller);
				}
			}

			ImGuiEx::CheckBox("启用原版死亡", &ExtraSettings.bShowGameKills, KFEnableVanillaKills);
			ImGui::SameLine();
			ImGuiEx::CheckBox("列出世界死亡", &ExtraSettings.bShowWorldKills, KFEnableWorldKills, !ExtraSettings.bShowGameKills);

		}
		ImGui::EndDisabled();

	}

	void DrawCommonWidgetOptions(auto& BaseSettings) {

		ImGui::SeparatorText("通用设置");

		ImGuiEx::CheckBox("启用", &BaseSettings.bVisible, TCVis);

		ImGui::BeginDisabled(!BaseSettings.bVisible);
		{
			// Anchor & Position
			ImGuiEx::ComboEx<GTS::ImWindow::WindowAnchor>("锚点", BaseSettings.sAnchor, TCAnchor);
			ImGuiEx::SliderF("位置 上/下", &BaseSettings.f2Position.at(1), 0.0f, 720.f, TCPosOffset, "%.1f");

			if (BaseSettings.sAnchor != "kCenter") {
				ImGuiEx::SliderF("位置 左/右", &BaseSettings.f2Position.at(0), 0.0f, 1280.f, TCPosOffset, "%.1f");
			}

			ImGui::Spacing();

			// Fade Settings
			ImGuiEx::CheckBox("无变化淡出", &BaseSettings.bEnableFade, TCIFadeEn);
			ImGuiEx::SliderF("淡出等待", &BaseSettings.fFadeAfter, 0.5f, 10.0f, TCIFadeTime, "%.1f 秒后", !BaseSettings.bEnableFade);
			ImGuiEx::SliderF("重新出现变化量", &BaseSettings.fFadeDelta, 0.0, 0.5f, TCIFadeDelta, "相差 %.2fx 后", !BaseSettings.bEnableFade);
			ImGuiEx::SliderF("透明度", &BaseSettings.fAlpha, 0.1f, 1.0f, TCAlpha, "%.2fx");
		}
		ImGui::EndDisabled();
	}

	template<typename SettingsType>
	void DrawCommonBarOptions(SettingsType& ExtraSettings) {
		// Toggles
		bool useGradient = ExtraSettings.iFlags & ImGuiEx::ImGuiExProgresbarFlag_Gradient;
		bool multiColor = ExtraSettings.iFlags & ImGuiEx::ImGuiExProgresbarFlag_DualColor;
		bool flipDir = ExtraSettings.iFlags & ImGuiEx::ImGuiExProgresbarFlag_SwapGradientDir;
		bool rounding = ExtraSettings.iFlags & ImGuiEx::ImGuiExProgresbarFlag_Rounding;

		ImGuiEx::SliderF("高度", &ExtraSettings.f2Size.at(1), 0.1f, 3.0f, TCHeight, "%.2fx");
		ImGuiEx::SliderF("长度", &ExtraSettings.f2Size.at(0), 50.0f, 700.0f, TCWidth, "%.0f");

		ImGui::Spacing();

		// Border Settings
		ImGuiEx::SliderF("边框粗细", &ExtraSettings.fBorderThickness, 0.0f, 5.0f, TCThickness, "%.2fx");
		ImGuiEx::SliderF("边框亮度", &ExtraSettings.fBorderLightness, 0.0f, 1.0f, TCBorderCol, "%.2fx");
		ImGuiEx::SliderF("边框透明度", &ExtraSettings.fBorderAlpha, 0.0f, 1.0f, TCBorderAlpha, "%.2fx");

		ImGui::Spacing();

		// Rounding
		if (ImGuiEx::CheckBox("圆角", &rounding, TCRoundingEn)) {
			ImUtil::ToggleFlag(ExtraSettings.iFlags, ImGuiEx::ImGuiExProgresbarFlag_Rounding, rounding);
		}

		if (rounding) {
			ImGuiEx::SliderF("圆角大小", &ExtraSettings.fRounding, 0.1f, 10.0f, TCRoundingAmt, "%.1fx");
		}

		ImGui::Spacing();

		// Gradient Settings
		if (ImGuiEx::CheckBox("渐变", &useGradient, TCGradientEn)) {
			ImUtil::ToggleFlag(ExtraSettings.iFlags, ImGuiEx::ImGuiExProgresbarFlag_Gradient, useGradient);
		}

		ImGui::BeginDisabled(!useGradient);
		{
			ImGui::SameLine();

			if (ImGuiEx::CheckBox("双色", &multiColor, TCDualColor)) {
				ImUtil::ToggleFlag(ExtraSettings.iFlags, ImGuiEx::ImGuiExProgresbarFlag_DualColor, multiColor);
			}

			ImGui::SameLine();

			if (ImGuiEx::CheckBox("反转方向", &flipDir, TCSwapDir)) {
				ImUtil::ToggleFlag(ExtraSettings.iFlags, ImGuiEx::ImGuiExProgresbarFlag_SwapGradientDir, flipDir);
			}
		}
		ImGui::EndDisabled();
	}

	template<typename SettingsType>
	void DrawColorOptions(SettingsType& ExtraSettings) {
		bool useGradient = ExtraSettings.iFlags & ImGuiEx::ImGuiExProgresbarFlag_Gradient;
		bool multiColor = ExtraSettings.iFlags & ImGuiEx::ImGuiExProgresbarFlag_DualColor;

		static GTS::ImGraphics::ImageTransform T = {
			.recolorEnabled = true,
		};
		T.targetColor = ImUtil::Colors::fRGBToImVec4(GTS::Config::UI.f3AccentColor);

		float buttonWidth = 18 + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x * 2;

		if (ImGuiEx::ImageButtonTransform("##ResetA", ImageList::Generic_Square, T, 18, TCopyAccent)) {
			ExtraSettings.f3ColorA = GTS::Config::UI.f3AccentColor;
		}
		ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
		ImGui::ColorEdit3("基础颜色", ExtraSettings.f3ColorA.data(), ImGuiColorEditFlags_DisplayHSV);

		if (useGradient) {
			if (multiColor) {
				if (ImGuiEx::ImageButtonTransform("##ResetB", ImageList::Generic_Square, T, 18, TCopyAccent)) {
					ExtraSettings.f3ColorB = GTS::Config::UI.f3AccentColor;
				}
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
				ImGui::ColorEdit3("渐变偏移颜色", ExtraSettings.f3ColorB.data(), ImGuiColorEditFlags_DisplayHSV);
			}
			else {
				ImGui::Text("单色渐变选项");
				ImGuiEx::SliderF("渐变亮度", &ExtraSettings.f2GradientRange.at(1), 1.0f, 3.0f, TCGradientIntens, "%.2fx");
				ImGuiEx::SliderF("渐变暗度", &ExtraSettings.f2GradientRange.at(0), 0.01f, 0.99f, TCGradientIntens, "%.2fx");
			}
		}
	}

	//Probably one of the most cursed things i've written for this mod...
	template<typename T, typename S>
	void DrawCommonOptionsFor(void* a_win) {
		if (!a_win) return;

		if (auto window = dynamic_cast<T*>(static_cast<T*>(a_win))) {
			DrawCommonWidgetOptions(window->GetBaseSettings());
		}
	}

	void DrawStatusBarOptions(GTS::ImWindow* a_targetBar) {

		if (!a_targetBar) return;
		auto Win = dynamic_cast<GTS::StatusBarWindow*>(a_targetBar);
		if (!Win) return;

		auto& BaseSettings = Win->GetBaseSettings();
		auto& ExtraSettings = Win->GetExtraSettings<WindowSettingsStatusBar_t>();

		ImGui::SeparatorText("扩展设置");

		ImGui::BeginDisabled(!BaseSettings.bVisible);
		{

			ImGuiEx::SliderU16("图标大小", &ExtraSettings.iIconSize, 8, 128, TIcoSize, "%d px");
			ImGuiEx::SliderF("相对字体缩放", &ExtraSettings.fRelativeFontScale, 0.5f, 1.5f, TRelativeFontScale, "%.1fx");

			bool sDamReduction   = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideDamageReduction);
			bool sLifeAbsorbtion = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideLifeAbsorbtion);
			bool sEnachantment   = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideEnchantment);
			bool sVoreStacks     = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideVoreStacks);
			bool sSizeReserve    = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideSizeReserve);
			bool sOnTheEdge      = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideOnTheEdge);
			bool sVoreAbsorbing  = !(ExtraSettings.iFlagsVis & ImGuiEx::StatusbarFlag_HideVoreBeingAbsorbed);

			bool ASDamReduction   = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASDamageReduction);
			bool ASLifeAbsorbtion = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASLifeAbsorbtion);
			bool ASEnachantment   = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASEnchantment);
			bool ASVoreStacks     = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASVoreStacks);
			bool ASSizeReserve    = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASSizeReserve);
			bool ASOnTheEdge      = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASOnTheEdge);
			bool ASVoreAbsorbing  = (ExtraSettings.iFlagsAS & ImGuiEx::StatusbarASFlag_ASVoreBeingAbsorbed);

			static GTS::ImGraphics::ImageTransform T = {
				.recolorEnabled = true,
			};

			T.targetColor = ImUtil::Colors::fRGBToImVec4(GTS::Config::UI.f3AccentColor);
			float buttonWidth = 18 + ImGui::GetStyle().ItemSpacing.x + ImGui::GetStyle().FramePadding.x * 2;
			if (ImGuiEx::ImageButtonTransform("##ResetA", ImageList::Generic_Square, T, 18, TCopyAccent)) {
				ExtraSettings.f3BGColor = GTS::Config::UI.f3AccentColor;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::CalcItemWidth() - buttonWidth);
			ImGui::ColorEdit3("背景颜色", ExtraSettings.f3BGColor.data(), ImGuiColorEditFlags_DisplayHSV);
			ImGuiEx::SliderF("背景透明度", &BaseSettings.fBGAlphaMult, 0.0f, 1.0f, TFrameAlpha, "%.2fx");
			ImGui::ColorEdit3("溢出颜色", GTS::Config::UI.f3IconOverflowColor.data(), ImGuiColorEditFlags_DisplayHSV);

			ImGui::Spacing();

			ImUtil_Unique
			{

				ImGui::Text("图标可见性开关");

				if (ImGuiEx::CheckBox("伤害减免", &sDamReduction, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideDamageReduction, !sDamReduction);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("生命吸收", &sLifeAbsorbtion, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideLifeAbsorbtion, !sLifeAbsorbtion);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("GTS 化身", &sEnachantment, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideEnchantment, !sEnachantment);
				}

				//------------ Line 2

				if (ImGuiEx::CheckBox("吞噬层数", &sVoreStacks, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideVoreStacks, !sVoreStacks);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("体型储备", &sSizeReserve, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideSizeReserve, !sSizeReserve);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("濒临边缘", &sOnTheEdge, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideOnTheEdge, !sOnTheEdge);
				}

				//------------ Line 3

				if (ImGuiEx::CheckBox("吞噬吸收中", &sVoreAbsorbing, TIcoVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsVis, ImGuiEx::StatusbarFlag_HideVoreBeingAbsorbed, !sVoreAbsorbing);
				}
			}

			// -------------- Always Show Toggles

			ImGui::Spacing();

			ImUtil_Unique 
			{

				ImGui::Text("始终显示图标开关");

				if (ImGuiEx::CheckBox("伤害减免", &ASDamReduction, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASDamageReduction, ASDamReduction);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("生命吸收", &ASLifeAbsorbtion, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASLifeAbsorbtion, ASLifeAbsorbtion);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("GTS 化身", &ASEnachantment, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASEnchantment, ASEnachantment);
				}

				//------------ Line 2

				if (ImGuiEx::CheckBox("吞噬层数", &ASVoreStacks, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASVoreStacks, ASVoreStacks);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("体型储备", &ASSizeReserve, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASSizeReserve, ASSizeReserve);
				}

				ImGui::SameLine();

				if (ImGuiEx::CheckBox("濒临边缘", &ASOnTheEdge, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASOnTheEdge, ASOnTheEdge);
				}

				//------------ Line 3

				if (ImGuiEx::CheckBox("吞噬吸收中", &ASVoreAbsorbing, TIcoAlwaysVis)) {
					ImUtil::ToggleFlag(ExtraSettings.iFlagsAS, ImGuiEx::StatusbarASFlag_ASVoreBeingAbsorbed, ASVoreAbsorbing);
				}
			}
		}
		ImGui::EndDisabled();
	}

	void DrawUnderstompBarOptions(GTS::ImWindow* a_targetBar) {
		if (!a_targetBar) return;
		auto Win = dynamic_cast<GTS::USBarWindow*>(a_targetBar);
		if (!Win) return;
		auto& BaseSettings = Win->GetBaseSettings();
		auto& ExtraSettings = Win->GetExtraSettings<WindowSettingsUnderstompBar_t>();

		ImGui::SeparatorText("扩展设置");
		ImGui::BeginDisabled(!BaseSettings.bVisible);
		{
			DrawCommonBarOptions(ExtraSettings);

			ImGuiEx::CheckBox("角度显示为倍率", &ExtraSettings.bShowScale, TShowUStompMult);
			ImGui::SameLine();
			ImGuiEx::CheckBox("绝对角度", &ExtraSettings.bShowAbsoluteAngle, TShowAbsAngle);

			DrawColorOptions(ExtraSettings);
		}
		ImGui::EndDisabled();
	}

	void DrawSizeBarOptions(GTS::ImWindow* a_targetBar) {

		if (!a_targetBar) return;
		auto Win = dynamic_cast<GTS::SizeBarWindow*>(a_targetBar);
		if (!Win) return;

		auto& BaseSettings = Win->GetBaseSettings();
		auto& ExtraSettings = Win->GetExtraSettings<WindowSettingsSizeBar_t>();

		ImGui::SeparatorText("扩展设置");

		ImGui::BeginDisabled(!BaseSettings.bVisible);
		{
			// Copy Player Style (only for non-player bars)
			if (!a_targetBar->GetWindowName().ends_with("P")) {
				if (ImGuiEx::Button("复制玩家样式", TCopyPlayer)) {
					if (const auto& P = dynamic_cast<GTS::SizeBarWindow*>(GTS::GTSMenu::WindowManager->wSBarP)) {
						const auto& B = P->GetBaseSettings();
						const auto& E = P->GetExtraSettings<WindowSettingsSizeBar_t>();

						BaseSettings.fAlpha = B.fAlpha;
						BaseSettings.bEnableFade = B.bEnableFade;
						BaseSettings.fFadeAfter = B.fFadeAfter;
						BaseSettings.fFadeDelta = B.fFadeDelta;

						ExtraSettings.fBorderLightness = E.fBorderLightness;
						ExtraSettings.fBorderAlpha = E.fBorderAlpha;
						ExtraSettings.fBorderThickness = E.fBorderThickness;
						ExtraSettings.f2Size = E.f2Size;
						ExtraSettings.fRounding = E.fRounding;
						ExtraSettings.f2GradientRange = E.f2GradientRange;
						ExtraSettings.iFlags = E.iFlags;
						ExtraSettings.bShowName = E.bShowName;
						ExtraSettings.bShowScale = E.bShowScale;
						ExtraSettings.bShowSize = E.bShowSize;
					}
				}
			}

			DrawCommonBarOptions(ExtraSettings);

			ImGuiEx::CheckBox("显示名称", &ExtraSettings.bShowName, TShowSName);
			ImGui::SameLine();
			ImGuiEx::CheckBox("显示倍率", &ExtraSettings.bShowScale, TShowSScale);
			ImGui::SameLine();
			ImGuiEx::CheckBox("显示体型", &ExtraSettings.bShowSize, TShowSSize);

			DrawColorOptions(ExtraSettings);
		}
		ImGui::EndDisabled();

	}

	

}

namespace GTS {

	CategoryWidgets::CategoryWidgets() {
		m_name = "组件";
	}

	void CategoryWidgets::DrawRight() {

		switch (IndexToDraw) {
			case 0: DrawSizeBarOptions(GTSMenu::WindowManager->wSBarP);  break;
			case 1: DrawSizeBarOptions(GTSMenu::WindowManager->wSBarF1); break;
			case 2: DrawSizeBarOptions(GTSMenu::WindowManager->wSBarF2); break;
			case 3: DrawSizeBarOptions(GTSMenu::WindowManager->wSBarF3); break;
			case 4: DrawSizeBarOptions(GTSMenu::WindowManager->wSBarF4); break;
			case 5: DrawSizeBarOptions(GTSMenu::WindowManager->wSBarF5); break;

			case 7: DrawUnderstompBarOptions(GTSMenu::WindowManager->wUBar);   break;
			case 8: DrawStatusBarOptions(GTSMenu::WindowManager->wStatusBar);  break;
			case 9: DrawKillFeedOptions(GTSMenu::WindowManager->wKillFeed);    break;

			default: break;

		}
	}

	void CategoryWidgets::DrawLeft() {

		switch (IndexToDraw) {

			case 0: DrawCommonOptionsFor<SizeBarWindow, WindowSettingsSizeBar_t>(GTSMenu::WindowManager->wSBarP);  break;
			case 1: DrawCommonOptionsFor<SizeBarWindow, WindowSettingsSizeBar_t>(GTSMenu::WindowManager->wSBarF1); break;
			case 2: DrawCommonOptionsFor<SizeBarWindow, WindowSettingsSizeBar_t>(GTSMenu::WindowManager->wSBarF2); break;
			case 3: DrawCommonOptionsFor<SizeBarWindow, WindowSettingsSizeBar_t>(GTSMenu::WindowManager->wSBarF3); break;
			case 4: DrawCommonOptionsFor<SizeBarWindow, WindowSettingsSizeBar_t>(GTSMenu::WindowManager->wSBarF4); break;
			case 5: DrawCommonOptionsFor<SizeBarWindow, WindowSettingsSizeBar_t>(GTSMenu::WindowManager->wSBarF5); break;

			case 7: DrawCommonOptionsFor<USBarWindow, WindowSettingsUnderstompBar_t>(GTSMenu::WindowManager->wUBar);       break;
			case 8: DrawCommonOptionsFor<StatusBarWindow, WindowSettingsStatusBar_t>(GTSMenu::WindowManager->wStatusBar);  break;
			case 9: DrawKillFeedWindowBase(GTSMenu::WindowManager->wKillFeed);  break;

			default: break;

		}
	}

	void CategoryWidgets::Draw() {
		DrawOptions();
		//Draw the split view
		ImCategorySplit::Draw();
	}

	void CategoryWidgets::DrawOptions() {
		ImGui::SetNextWindowBgAlpha(0.1f);
		static std::vector<std::string> SizebarList{
			"体型条 - 玩家",
			"体型条 - 追随者 1",
			"体型条 - 追随者 2",
			"体型条 - 追随者 3",
			"体型条 - 追随者 4",
			"体型条 - 追随者 5"
		};

		static std::vector<std::string> OtherWidgetList{
			"体型条",
			"脚下踩踏条",
			"状态栏",
			"击杀提示"
		};

		SizebarList[0] = fmt::format("体型条 - {}", PlayerCharacter::GetSingleton()->GetName());
		static std::string CurrentSizebar = SizebarList[0];
		static std::string CurrentOther = OtherWidgetList[0];

		const auto& tmplist = GTSMenu::WindowManager->GetCachedTeamMateList();
		for (uint8_t i = 1; i <= 5; i++) {
			if (i - 1 < tmplist.size() && tmplist[i - 1]) {
				SizebarList[i] = fmt::format("体型条 - {}", tmplist[i - 1]->GetName());
			}
			else {
				SizebarList[i] = fmt::format("体型条 - 追随者 {}", i);
			}
		}

		ImGui::BeginChild("##Options", { -FLT_MIN, 0.0f }, ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_FrameStyle);
		{
			if (ImGui::BeginCombo("选择组件", CurrentOther.c_str())) {
				for (size_t i = 0; i < OtherWidgetList.size(); ++i) {
					bool selected = (CurrentOther == OtherWidgetList[i]);
					if (ImGui::Selectable(OtherWidgetList[i].c_str(), selected)) {
						CurrentOther = OtherWidgetList[i];
						if (i == 0) { // "Sizebar" selected
							IndexToDraw = 0; // Default to player sizebar
						}
						else {
							IndexToDraw = 6 + i;
						}
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}

			ImGuiEx::SeperatorV();

			if (CurrentOther == "体型条") {

				if (ImGui::BeginCombo("选择体型条", CurrentSizebar.c_str())) {
					for (size_t i = 0; i < SizebarList.size(); ++i) {
						bool selected = (CurrentSizebar == SizebarList[i]);
						if (ImGui::Selectable(SizebarList[i].c_str(), selected)) {
							CurrentSizebar = SizebarList[i];
							IndexToDraw = i;
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
			}
		}
		ImGui::EndChild();
	}
}
