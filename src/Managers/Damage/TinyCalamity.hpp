#pragma once

namespace GTS {

    bool TinyCalamity_WrathfulCalamity(Actor* giant);
    
    void TinyCalamity_ShrinkActor(Actor* giant, Actor* tiny, float shrink);
    void TinyCalamity_SeekForShrink(Actor* giant, Actor* tiny, float damage, float maxFootDistance, DamageSource Cause, bool Right, bool ApplyCooldown, bool ignore_rotation);
    void TinyCalamity_ExplodeActor(Actor* giant, Actor* tiny);
    void TinyCalamity_StaggerActor(Actor* giant, Actor* tiny, float giantHp);
     
    void TinyCalamity_SeekActors(Actor* giant, const std::vector<Actor*>& actors);
    void TinyCalamity_CrushCheck(Actor* giant, Actor* tiny);
    void TinyCalamity_BonusSpeed(Actor* giant);
}
