#include "UI/Controls/Icons/DynIcon_Cooldown_BreastSuffocate.hpp"

namespace ImGuiEx {

	DynIconCooldownBreastSuffocate::DynIconCooldownBreastSuffocate(uint32_t a_size) : IDynIcon(ImageList::CooldownIcon_BreastSuffocate, a_size) {}

	bool DynIconCooldownBreastSuffocate::Draw(float a_percent, float a_totalCooldown, bool a_alwaysShow) const {

		if ((a_percent <= 0.1f || a_percent == a_totalCooldown) && !a_alwaysShow) return false;
		return DrawIcon(a_percent, a_totalCooldown * 0.8f, fmt::format("{:.1f}s", a_percent));
	}
}
