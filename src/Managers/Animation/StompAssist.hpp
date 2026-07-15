#pragma once

namespace GTS {

	enum class StompAssistAction {
		Normal,
		Strong,
		Trample,
	};

	// preferredTarget: when set, only that actor is assisted (used by AI stomp start).
	void TryStompAssist(Actor* giant, bool right, StompAssistAction action, Actor* preferredTarget = nullptr);
	bool IsStompAssistActive(Actor* actor);
	// Same formula as CollectTargets: radius * 1.6 * visual scale.
	float GetStompAssistSearchDistance(Actor* giant);
	float GetStompAssistSizeThreshold();
}
