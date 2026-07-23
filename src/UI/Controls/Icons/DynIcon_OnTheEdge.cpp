#include "UI/Controls/Icons/DynIcon_OnTheEdge.hpp"

namespace ImGuiEx {

	DynIconOnTheEdge::DynIconOnTheEdge(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_OnTheEdge, a_size) {}

	bool DynIconOnTheEdge::Draw(float a_percent, bool a_alwaysShow) const {

		if (a_percent - 1.0f <= 0.f && !a_alwaysShow) return false;
		return DrawIcon(a_percent, 60.f, fmt::format("{:.1f}%", a_percent));
	}
}
