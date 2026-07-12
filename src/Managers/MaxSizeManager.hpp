#pragma once

#include "Config/Settings/SettingsBalance.hpp"

namespace GTS {
    bool ActorMatchesSizeLimitRuleTarget(Actor* a_actor, LSizeLimitRuleTarget_t a_target);
    void EnsureSizeLimitRulesInitialized();
    bool SizeLimitRulesActive();
    void UpdateMaxScale(Actor* a_actor, PersistentActorData* a_persistent = nullptr, TransientActorData* a_transient = nullptr);

    void VisualScale_CheckForSizeAdjustment(Actor* actor, float& ScaleMult);
    void Ench_Potions_ApplyBonuses(Actor* actor, float& value);

    float GetActionCompatibleSizeLimit();
    float MassMode_GetValuesForMenu(Actor* actor);

}
