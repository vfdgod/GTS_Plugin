#include "UI/Controls/Icons/DynIcon_VoreBeingAbsorbed.hpp"

namespace ImGuiEx {

	DynIconVoreBeingAbsorbed::DynIconVoreBeingAbsorbed(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_VoreBeingAbsorbed, a_size) {}

	bool DynIconVoreBeingAbsorbed::Draw(int a_stacks, bool a_alwaysShow) const {

		if (a_stacks <= 0 && !a_alwaysShow) return false;
		return DrawIcon(a_stacks, 10, fmt::format("{:d}x", a_stacks));
	}
}
