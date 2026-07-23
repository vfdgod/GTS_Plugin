#include "UI/Controls/Icons/DynIcon_Cooldown_BreastAbsorb.hpp"

namespace ImGuiEx {
	
	DynIconCooldownBreastAbsorb::DynIconCooldownBreastAbsorb(uint32_t a_size) : IDynIcon(ImageList::CooldownIcon_BreastAbsorb, a_size) {}

	bool DynIconCooldownBreastAbsorb::Draw(float a_percent, float a_totalCooldown, bool a_alwaysShow) const {

		if ((a_percent <= 0.1f || a_percent == a_totalCooldown) && !a_alwaysShow) return false;
		return DrawIcon(a_percent, a_totalCooldown * 0.8f, fmt::format("{:.1f}s", a_percent));
	}
}
