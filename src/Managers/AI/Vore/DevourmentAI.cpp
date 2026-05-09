#include "Managers/AI/Vore/DevourmentAI.hpp"

#include "Utils/Actions/ActionUtils.hpp"

namespace {

	constexpr float BASE_CONE_WIDTH = 70.0f;
	constexpr float VORE_ANGLE = 75.0f;
	constexpr float MINIMUM_DISTANCE = 60.0f;
}


namespace GTS {

	static bool VoreAI_CanVore(Actor* a_Pred, Actor* a_Prey) {

		if (a_Pred == a_Prey) {
			return false;
		}

		if (IsBeingHeld(a_Pred, a_Prey)) {
			return false;
		}

		const auto Transient = Transient::GetActorData(a_Prey);
		if (Transient) {
			if (Transient->CanBeVored == false) {
				return false;
			}
		}

		const float PredScale = get_visual_scale(a_Pred);
		const float PreyDistance = (a_Pred->GetPosition() - a_Prey->GetPosition()).Length();


		if (PreyDistance <= MINIMUM_DISTANCE * PredScale) {
			return true;
		}

		return false;
	}

	std::vector<Actor*> DevourmentAI_FilterList(Actor* a_Pred, const std::vector<Actor*>& a_PotentialPrey) {

		// Get vore target for actor
		if (!a_Pred) {
			return {};
		}

		auto CharController = a_Pred->GetCharController();
		if (!CharController) {
			return {};
		}

		return SelectTargetsInFront(a_Pred, a_PotentialPrey, a_PotentialPrey.size(), VORE_ANGLE, true, [a_Pred](auto prey) {
			return VoreAI_CanVore(a_Pred, prey);
		});
	}

	void DevourmentAI_Start(Actor* a_Predator, const std::vector<Actor*>& a_PotentialPrey) {
		DamageAV(a_Predator, ActorValue::kStamina, 30.0f);
		CallDevourment(a_Predator, a_PotentialPrey.front());
	}
}



