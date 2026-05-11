#pragma once

#include "Config/Settings/SettingsBalance.hpp"

namespace GTS {
    bool ActorMatchesSizeLimitRuleTarget(Actor* a_actor, LSizeLimitRuleTarget_t a_target);
    void EnsureSizeLimitRulesInitialized();
    void UpdateMaxScale(Actor* a_actor);
    void UpdateGlobalSizeLimit(Actor* a_actor);

    void VisualScale_CheckForSizeAdjustment(Actor* actor, float& ScaleMult);
    void Ench_Potions_ApplyBonuses(Actor* actor, float& value);

    float GetActionCompatibleSizeLimit(bool a_forceRefresh = false);
    float GetExpectedMaxSize(RE::Actor* a_Actor, float start_value = 1.0f);
    float MassMode_GetValuesForMenu(Actor* actor);

}
