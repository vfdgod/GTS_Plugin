#pragma once

#include "Systems/Events/EventData.hpp"

// Module for accurate size-related damage

namespace GTS {

	class CollisionDamage  {
		public:
		static void DoFootCollision(Actor* actor, float damage, float radius, int random, float bbmult, float crush_threshold, DamageSource Cause, bool right, bool ApplyCooldown, bool ignore_rotation, bool SupportCalamity, FootActionDamageLimitKind damage_limit_kind = FootActionDamageLimitKind::None);
		static void DoSizeDamage(Actor* giant, Actor* tiny, float damage, float bbmult, float crush_threshold, int random, DamageSource Cause, bool apply_damage, FootActionDamageLimitKind damage_limit_kind = FootActionDamageLimitKind::None);
		static void CrushCheck(Actor* giant, Actor* tiny, float size_difference, float crush_threshold, DamageSource Cause);
	};

}
