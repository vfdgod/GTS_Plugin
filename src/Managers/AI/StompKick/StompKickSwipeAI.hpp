#pragma once


namespace GTS {
	// a_UseStompAssistFix: AI 踩踏修复用辅助半径/体型；踢击应始终传 false 走原前方锥逻辑。
	std::vector<Actor*> StompKickSwipeAI_FilterList(Actor* a_Pred, const std::vector<Actor*>& a_PotentialPrey, bool a_UseStompAssistFix = false);
	void StompAI_Start(Actor* a_Performer, Actor* a_Prey);
	void KickSwipeAI_Start(Actor* a_Performer);
}
