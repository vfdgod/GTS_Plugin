#pragma once

namespace GTS {
    //-----------------------------------------------------
	// FOOT OR HAND AUTO AIM
	//-----------------------------------------------------
	bool AutoAim_Kick_DeterminePreferredKick(Actor* giant);
	bool AutoAim_Butt_TryBreastSlam(Actor* giant, bool& left_hand);
	bool AutoAim_Butt_TryButtSlam(Actor* giant, bool& left_hand);
	bool AutoAim_Hand_TryHandAim(Actor* giant, bool& left_hand);
	bool AutoAim_Foot_Directional(Actor* giant, bool& left_foot, bool forward_only);
	bool AutoAim_Foot_Directional_FarStomp(Actor* giant, bool& left_foot, bool strong_stomp);
	bool AutoAim_Foot_CanTarget(Actor* giant, Actor* target, bool include_far_stomp);
	bool AutoAim_Foot_Directional_OnTarget(Actor* giant, Actor* target, bool& left_foot, bool forward_only);
	bool AutoAim_Foot_Directional_FarStomp_OnTarget(Actor* giant, Actor* target, bool& left_foot, bool strong_stomp);
	bool AutoAim_IsSneakingOrCrawling(Actor* giant);
	bool AutoAim_SetUpDefaultSide(Actor* giant);
}
