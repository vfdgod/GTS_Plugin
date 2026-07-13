#pragma once
#pragma pack(push, 1)
namespace GTS {

	struct PersistentActorData {

		//Scale
		float fVisualScale = 1.0f;
		float fVisualScaleV = 0.0f;
		float fTargetScale = 1.0f;
		float fTargetScaleV = 0.0f;
		float fMaxScale = 65535.0f;
		float fExtraPotionMaxScale = 0.0f;
		float fSizeLimit = 1.0f;
		float fMassBasedLimit = 0.0f;

		//Damage
		float fNormalDamage = 1.0f;
		float fSprintDamage = 1.0f;
		float fFallDamage = 1.0f;
		float fHHDamage = 1.0f;

		//Perks
		float fSMTRunSpeed = 0.0f;
		float fSizeReserve = 0.0f;
		float fStolenAttibutes = 0.0f;
		float fStolenHealth = 0.0f;
		float fStolenMagicka = 0.0f;
		float fStolenStamina = 0.0f;

		//Other
		float fHalfLife = 1.0f;
		float fAnimSpeed = 1.0f;

		// 每名追随者独立保存的微型灾厄模拟设置。
		// TLV 序列化会忽略旧 co-save 中不存在的字段，因此可向后兼容。
		bool bTinyCalamitySimulationActive = false;
		bool bTinyCalamitySimulationSprintBoost = false;
		bool bTinyCalamitySimulationActionBoost = false;
		bool bTinyCalamitySimulationIgnoreNaturalFootIdle = true;
		bool bTinyCalamitySimulationShrinkBoost = false;
		bool bTinyCalamitySimulationAttributeBoost = false;
		bool bTinyCalamitySimulationRefresh = false;
		bool bTinyCalamitySimulationAug = false;
		bool bTinyCalamitySimulationSizeSteal = false;
		bool bTinyCalamitySimulationRage = false;
		bool bTinyCalamitySimulationShrinkingGaze = false;

		//Voice
		bool bUseSLVoice = false;

		//Morphs
		float fBreastsScale = 0.0f;
		float fBellyScale = 0.0f;

		//Skill Level (NPC only)
		float fGTSSkillLevel = 0.0f;
		float fGTSSkillExp   = 0.0f;
		float fGTSSkillRatio = 0.0f;

		/*
		//These two should trigger a hash colission
		int costarring;  
		int liquid;
		*/

	};


	struct PersistentKillCountData {

		uint32_t iTotalKills = 0;      // Total kill count when we don't expand all the info
		uint32_t iShrunkToNothing = 0; // Mostly with spells
		uint32_t iOtherSources = 0;    // Things like Colliding with someone with Tiny Calamity (leads to exploding the tiny) 
		                               // or inflicting too much damage with weapons when size difference is gigantic (also explodes tiny)
		// Breast
		uint32_t iBreastAbsorbed = 0;
		uint32_t iBreastCrushed = 0;
		uint32_t iBreastSuffocated = 0;

		// Hug
		uint32_t iHugCrushed = 0;

		// Grab Data
		uint32_t iGrabCrushed = 0;

		// Butt Crush Data
		uint32_t iButtCrushed = 0;

		//Thigh Sandwich/Crush
		uint32_t iThighCrushed = 0;
		uint32_t iThighSuffocated = 0;     // When dying from DOT damage under thighs
		uint32_t iThighSandwiched = 0;     // When dying from Thigh Sandwich

		//Other
		uint32_t iThighGrinded = 0;        // We with Nick plan Thigh Grind anim, so it may be used later
		uint32_t iFingerCrushed = 0;
		uint32_t iErasedFromExistence = 0; // 狂怒灾厄处决
		uint32_t iAbsorbed = 0;            // Absorbed through size-related effects
		uint32_t iCrushed = 0;             // Used in most crush sources
		uint32_t iEaten = 0;               // When fully voring someone
		uint32_t iKicked = 0;              // Kicked and crushed to death at same time
		uint32_t iGrinded = 0;             // Grinded to death

	};
}

#pragma pack(pop)
