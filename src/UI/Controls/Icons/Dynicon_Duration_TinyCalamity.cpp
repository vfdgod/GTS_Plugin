#include "UI/Controls/Icons/DynIcon_Duration_TinyCalamity.hpp"

namespace ImGuiEx {
	
	DynIconDurationTinyCalamity::DynIconDurationTinyCalamity(uint32_t a_size) : IDynIcon(ImageList::DurationIcon_CalamityDuration, a_size) {}

	bool DynIconDurationTinyCalamity::Draw(float a_percent, float a_totalCooldown, bool a_alwaysShow) const {

		if (a_percent <= 0.1f && !a_alwaysShow) return false;
		const std::string text = a_percent > 900.0f ? "∞" : fmt::format("{:.1f}s", a_percent);
		return DrawIcon(a_percent, a_totalCooldown * 0.5f, text);
	}
}
