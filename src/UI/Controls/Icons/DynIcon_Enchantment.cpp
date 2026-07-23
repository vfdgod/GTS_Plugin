#include "UI/Controls/Icons/DynIcon_Enchantment.hpp"

namespace ImGuiEx {

	DynIconEnchantment::DynIconEnchantment(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_GTSAspect, a_size) {}

	bool DynIconEnchantment::Draw(float a_percent, bool a_alwaysShow) const {

		if (a_percent - 1.0f <= 0.f && !a_alwaysShow) return false;
		return DrawIcon(a_percent, 150.f, fmt::format("{:.1f}%", a_percent));
	}
}
