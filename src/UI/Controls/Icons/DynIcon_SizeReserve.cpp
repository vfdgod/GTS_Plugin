#include "UI/Controls/Icons/DynIcon_SizeReserve.hpp"

namespace ImGuiEx {

	DynIconSizeReserve::DynIconSizeReserve(uint32_t a_size) : IDynIcon(ImageList::BuffIcon_SizeReserve, a_size) {}

	bool DynIconSizeReserve::Draw(float a_amount, bool a_alwaysShow) const {

		if (a_amount <= 0 && !a_alwaysShow) return false;
		return DrawIcon(a_amount, 10.0f, fmt::format("{:.2f}x", a_amount));
	}
}
