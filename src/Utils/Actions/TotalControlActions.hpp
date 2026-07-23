#pragma once

namespace GTS::TotalControlActions {

	void GrowTeammatesOverTime(float a_power);
	void ShrinkTeammatesOverTime(float a_power);
	void GrowPlayerOverTime(float a_power);
	void ShrinkPlayerOverTime(float a_power, float a_minimumScale);
}
