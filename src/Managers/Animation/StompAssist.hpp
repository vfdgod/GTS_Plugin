#pragma once

namespace GTS {

	enum class StompAssistAction {
		Normal,
		Strong,
		Trample,
	};

	void TryStompAssist(Actor* giant, bool right, StompAssistAction action);
	bool IsStompAssistActive(Actor* actor);
}
