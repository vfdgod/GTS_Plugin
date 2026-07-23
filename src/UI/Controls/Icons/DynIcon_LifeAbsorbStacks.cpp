#include "UI/Controls/Icons/DynIcon_LifeAbsorbStacks.hpp"

namespace ImGuiEx {

	DynIconLifeabsorbStacks::DynIconLifeabsorbStacks(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_LifeAbsorbStacks, a_size) {}

	bool DynIconLifeabsorbStacks::Draw(int a_stacks, bool a_alwaysShow) const {

		if (a_stacks <= 0 && !a_alwaysShow) return false;
		return DrawIcon(a_stacks, 25, fmt::format("{:d}x", a_stacks));
	}
}
