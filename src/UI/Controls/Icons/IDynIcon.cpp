#include "UI/Controls/Icons/IDynIcon.hpp"

#include "Config/Config.hpp"
#include "UI/Controls/Text.hpp"
#include "UI/Core/ImColorUtils.hpp"
#include "UI/Core/ImFontManager.hpp"
#include "UI/Core/ImUtil.hpp"
#include "UI/GTSMenu.hpp"
#include "UI/Windows/Widgets/StatusBarWindow.hpp"

namespace ImGuiEx {

	bool IDynIcon::DrawIcon(float a_value, float a_overflow, const std::string& a_text) const {
		float relativeFontScale = 1.0f;
		if (const auto* manager = GTS::GTSMenu::WindowManager.get()) {
			if (const auto* statusBar = dynamic_cast<GTS::StatusBarWindow*>(manager->wStatusBar)) {
				relativeFontScale = statusBar->GetExtraSettings<WindowSettingsStatusBar_t>().fRelativeFontScale;
			}
		}

		GTS::ImFontManager::Push(
			GTS::ImFontManager::kIconText,
			(static_cast<float>(m_size) / static_cast<float>(m_referenceSize)) * relativeFontScale
		);
		struct FontGuard {
			~FontGuard() { GTS::ImFontManager::Pop(); }
		} fontGuard;

		const bool validOverflow = std::isfinite(a_overflow) && a_overflow > 0.0f;
		m_transform->targetColor = validOverflow && a_value > a_overflow ?
			ImUtil::Colors::fRGBToImVec4(GTS::Config::UI.f3IconOverflowColor) :
			ImVec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		m_transform->gradientStartPercent = validOverflow ? std::clamp(a_value / a_overflow, 0.0f, 1.0f) : 0.0f;
		m_transform->gradientTargetAlpha = 0.0f;

		const float iconSize = static_cast<float>(m_size);
		if (!GTS::ImGraphics::RenderTransformed(m_name, *m_transform, { iconSize, iconSize })) {
			return false;
		}

		const auto pos = ImUtil::GetCenteredTextPos(ImGui::GetItemRectMin(), ImGui::GetItemRectSize(), a_text.c_str());
		TextShadowImplExDummy(pos, a_text.data(), a_text.data() + a_text.size());
		return true;
	}
}
