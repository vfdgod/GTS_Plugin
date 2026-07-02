#pragma once
#include "Config/Util/TomlRefl.hpp"

/*
Reflection based serializer limitations:

 - C style arrays are unsupported so no "char foo[32]" for example
 - std::array arrays or std::vectors however do work with basic types (aggregate types are untested)
 - Data types like std::tuple or ordered map are unsupported (others are untested)
 - Nested structs are supported as long as these also are put through the TOML_SERIALIZABLE() macro.
 - Structs can only contain a total of 64 unique entries, this is a limitation of the reflect library.
   In order to be able to have > 64 the visit template in the reflect library needs to be expanded.
 - Enums are also unsupported. They can however be saved as either int or string. Its better to save them as a string though
   and use something like magic_enum to do the conversion between string <-> enum.
*/

/* Naming Convention
* i(x) -> integer(Ammount of elements if array)
* f(x) -> float(Ammount of elements if array)
* b(x) -> bool(Ammount of elements if array)
* s(x) -> string(Ammount of elements if array)
* stucts don't get a special notation
*/

//-------------------------------------------------------------------------------------------------------------------
//  BASE STRUCT
//  (Directly Serialized)
//-------------------------------------------------------------------------------------------------------------------

struct SettingsAdvanced_t {

    enum class LShrinkApplicationMode_t : uint8_t {
        kGradual = 0,
        kInstant,
        kTotal
    };

    // Logging levels
    std::string sLogLevel = "err";
    std::string sShrinkMode = "kGradual";

    // Toggles for advanced features
    bool bShowOverlay = false;
    bool bDamageAV = true;
    bool bCooldowns = true;
    bool bEnforceUIClamps = true;
    bool bPlayerTinyCalamityActive = false;
    bool bPlayerTinyCalamitySprintBoost = false;
    bool bPlayerTinyCalamityActionBoost = false;
    bool bPlayerTinyCalamityShrinkBoost = false;
    bool bPlayerTinyCalamityAttributeBoost = false;
    bool bPlayerTinyCalamityRefresh = false;
    bool bPlayerTinyCalamityAug = false;
    bool bPlayerTinyCalamitySizeSteal = false;
    bool bPlayerTinyCalamityRage = false;
    bool bPlayerTinyCalamityShrinkingGaze = false;
    bool bPlayerFootAutoAim = false;
    bool bStompAssist = false;
    bool bStompAssistNormal = true;
    bool bStompAssistStrong = true;
    bool bStompAssistTrample = true;
    bool bStompAssistMultiTarget = false;
    uint8_t iStompAssistMaxTargets = 3;
    float fStompAssistDuration = 1.0f;
    float fStompAssistSearchRadius = 36.0f;
    float fStompAssistSizeThreshold = 3.5f;
    bool bFootActionDamageLimit = false;
    bool bFootActionDamageLimitNormal = true;
    bool bFootActionDamageLimitStrong = true;
    bool bFootActionDamageLimitTrample = true;
    float fFootActionDamageLimitMaxHealthPercent = 45.0f;
    bool bPlayerAI = false;
    float fAnimSpeedAdjMultPlayer = 1.0f;
    float fAnimSpeedAdjMultTeammate = 1.0f;
    bool bEnableExperimentalDevourmentAI = false;
    float fExperimentalDevourmentAIProb = 25.0f;
    bool bShareSettingsGlobally = true;
    std::array<float, 5> fAnimSpeedSoftCore = 
    {
        // https://www.desmos.com/calculator/vyofjrqmrn
	    0.140f, //.k = 0.142f, 
    	0.540f,  //.n = 0.82f,
    	1.350f,  //.s = 1.90f, 
    	1.0f,   //.o = 1.0f,
    	0.0f    //.a = 0.0f,  //Default is 0
    };

    bool bGTSAnimsFullSpeed = false;
    float fAnimspeedLowestBoundAllowed = 0.01f;

};
TOML_SERIALIZABLE(SettingsAdvanced_t);
TOML_REGISTER_NAME(SettingsAdvanced_t, "Advanced");
