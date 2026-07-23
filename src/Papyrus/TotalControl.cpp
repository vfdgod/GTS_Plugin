#include "Papyrus/TotalControl.hpp"
#include "Magic/Effects/Common.hpp"
#include "Utils/Actions/TotalControlActions.hpp"

using namespace GTS;
using namespace RE::BSScript;

namespace {

	constexpr std::string_view PapyrusClass = "GTSControl";

	bool CanUseTotalControl(float power) {
		Actor* player = PlayerCharacter::GetSingleton();
		return power > 0.0f && player && Runtime::HasPerkTeam(player, Runtime::PERK.GTSPerkGrowthDesireAug);
	}
	void GrowTeammate(StaticFunctionTag*, float power) {
		if (CanUseTotalControl(power)) {
			TotalControlActions::GrowTeammatesOverTime(power);
		}
	}

	void ShrinkTeammate(StaticFunctionTag*, float power) {
		if (CanUseTotalControl(power)) {
			TotalControlActions::ShrinkTeammatesOverTime(power);
		}
	}

	void GrowPlayer(StaticFunctionTag*, float power) {
		if (CanUseTotalControl(power)) {
			TotalControlActions::GrowPlayerOverTime(power);
		}
	}

	void ShrinkPlayer(StaticFunctionTag*, float power) {
		if (CanUseTotalControl(power)) {
			TotalControlActions::ShrinkPlayerOverTime(power, Minimum_Actor_Scale);
		}
	}

	void CallRapidGrowth(StaticFunctionTag*, float amt, float halflife) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			if (Runtime::HasPerkTeam(player, Runtime::PERK.GTSPerkGrowthDesireAug)) {
				float target = get_target_scale(player);
				float max_scale = get_max_scale(player);// * get_natural_scale(player);
				if (target >= max_scale) {
					NotifyWithSound(player, "You can't grow any further");
					shake_camera(player, 0.45f, 0.30f);
					return;
				}
				SpringGrow(player, amt, halflife, "Input_G", true);
			}
		}
	}

	void CallRapidShrink(StaticFunctionTag*, float amt, float halflife) {
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			if (Runtime::HasPerkTeam(player, Runtime::PERK.GTSPerkGrowthDesireAug)) {
				float target = get_target_scale(player);
				if (target <= Minimum_Actor_Scale) {
					NotifyWithSound(player, "You can't shrink any further");
					shake_camera(player, 0.45f, 0.30f);
					return;
				}
				SpringShrink(player, amt, halflife, "Input_S");

			}
		}
	}

}

namespace GTS {

	bool register_total_control(IVirtualMachine* vm) {

		//Followers
		vm->RegisterFunction("GrowTeammate", PapyrusClass, GrowTeammate);
		vm->RegisterFunction("ShrinkTeammate", PapyrusClass, ShrinkTeammate);

		//Player
		vm->RegisterFunction("GrowPlayer", PapyrusClass, GrowPlayer);
		vm->RegisterFunction("ShrinkPlayer", PapyrusClass, ShrinkPlayer);

		//Rappid Growth
		vm->RegisterFunction("CallRapidGrowth", PapyrusClass, CallRapidGrowth);
		vm->RegisterFunction("CallRapidShrink", PapyrusClass, CallRapidShrink);

		return true;
	}

}
