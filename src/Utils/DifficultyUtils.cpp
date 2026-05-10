#include "Utils/DifficultyUtils.hpp"

using namespace GTS;

namespace {

    constexpr std::array<const char* const, 6> Difficulty_ByPC {
        "fDiffMultHPByPCVE",
        "fDiffMultHPByPCE",
        "fDiffMultHPByPCN",
        "fDiffMultHPByPCH",
        "fDiffMultHPByPCVH",
        "fDiffMultHPByPCL"
    };

    constexpr std::array<const char* const, 6> Difficulty_ToPC {
        "fDiffMultHPToPCVE",
        "fDiffMultHPToPCE",
        "fDiffMultHPToPCN",
        "fDiffMultHPToPCH",
        "fDiffMultHPToPCVH",
        "fDiffMultHPToPCL",
    };
}

namespace GTS {

    float GetSettingValue(const char* setting) {
		float modifier = 1.0f;
		auto GameSetting = GameSettingCollection::GetSingleton();
		if (GameSetting) {
			modifier = GameSetting->GetSetting(setting)->GetFloat();
		}
		//log::info("Difficulty Modifier: {}", modifier);
		return modifier;
	}

	float GetDifficultyMultiplier(Actor* attacker, Actor* receiver) { // Credits to Doodlum for this method
		const auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return 1.0f;
		}

		const auto currentdiff = static_cast<std::size_t>(player->GetGameStatsData().difficulty);
		if (currentdiff >= Difficulty_ByPC.size() || currentdiff >= Difficulty_ToPC.size()) {
			return 1.0f;
		}

		if (attacker && (attacker->IsPlayerRef() || IsTeammate(attacker))) {
            //log::info("Current By PC Difficulty: {}", Difficulty_ByPC[currentdiff]);
			return GetSettingValue(Difficulty_ByPC[currentdiff]);
		}
		else if (receiver && (receiver->IsPlayerRef() || IsTeammate(receiver))) {
            //log::info("Current To PC Difficulty: {}", Difficulty_ToPC[currentdiff]);
			return GetSettingValue(Difficulty_ToPC[currentdiff]);
		}
		return 1.0f;
	}
}
