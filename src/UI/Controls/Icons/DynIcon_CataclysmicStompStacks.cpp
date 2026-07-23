#include "UI/Controls/Icons/DynIcon_CataclysmicStompStacks.hpp"

namespace ImGuiEx {

	DynIconCataclysmicStompStacks::DynIconCataclysmicStompStacks(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_StompStacks, a_size) {}

	bool DynIconCataclysmicStompStacks::Draw(int a_stacks, bool a_alwaysShow) const {

		if (a_stacks <= 0 && !a_alwaysShow) return false;
		return DrawIcon(a_stacks, 3, fmt::format("{:d}x", a_stacks));
	}
}
