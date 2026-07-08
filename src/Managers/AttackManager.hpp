#pragma once

namespace GTS {

	class AttackManager {
		public:
		static bool ShouldBlockShrunkAttacks(Actor* a_Actor);
		static void SetAttacksDisabled(Actor* a_Actor, bool a_Disabled);
		static void PreventAttacks(Actor* a_Giant, Actor* a_Tiny);
	};
}
