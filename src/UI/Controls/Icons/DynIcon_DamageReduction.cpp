#include "UI/Controls/Icons/DynIcon_DamageReduction.hpp"

namespace ImGuiEx {

	DynIconDamageReduction::DynIconDamageReduction(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_DamageReduction, a_size) {}

	bool DynIconDamageReduction::Draw(float a_percent, bool a_alwaysShow) const {

		if (a_percent - 1.0f <= 0.f && !a_alwaysShow) return false;
		return DrawIcon(a_percent, 95.f, fmt::format("{:.1f}%", a_percent));
	}
}
