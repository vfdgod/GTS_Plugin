#pragma once

// Handles the various methods of scaling an actor

namespace GTS {

	// @ Sermit, do not call Get_Other_Scale, call get_natural_scale instead
	// get_natural_scale is much faster and safer as it uses the cache
	float Get_Other_Scale(Actor* actor);

	// Initial scale is cached for the currently loaded game and cleared between saves.
	// This preserves ESP/NIF scale edits without leaking runtime data into another save.
	float GetInitialScale(Actor* actor);
	void ClearInitialScales();
	// Should be called on save load and on swapping the scale mode
	void ResetToInitScale(Actor* actor);

	void RefreshInitialScales(Actor* actor);
	bool set_model_scale(Actor* actor, float target_scale);
	bool set_npcnode_scale(Actor* actor, float target_scale);

	float get_npcnode_scale(Actor* actor);
	float get_npcparentnode_scale(Actor* actor);
	float get_model_scale(Actor* actor);
	float get_scale(Actor* actor);

	float game_getactorscale(Actor* actor);
	float game_getactorscale(Actor& actor);

	float game_get_scale_overrides(Actor* actor);

	float game_get_scale_overrides(Actor& actor);
	
	bool update_model_visuals(Actor* actor, float scale);
}
