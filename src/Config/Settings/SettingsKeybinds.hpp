#pragma once
#include "Config/Util/TomlRefl.hpp"

//-------------------------------------------------------------------------------------------------------------------
//  ENUMS ----- Assumed to be the reference values 
//  magic_enum will use to convert an enum to a string representation for serialization (Saving The TOML)
//-------------------------------------------------------------------------------------------------------------------

//Don't forget to add the entries to CategoryNames as well if adding any new ones.
enum class LInputCategory_t : uint8_t {
	kDefault = 0,
	kMenu,
	kVore,
	kHugs,
	kThighs,
	kGrab,
	kBreats,
	kGrabPlay,
	kStomp,
	kKickSwipe,
	kMovement,
	kCrush,
	kCleavage,
	kCamera,
	kAbility,
	kMisc,

	kTotal
};



enum class LTriggerType_t : uint8_t {
	Once,
	Continuous,
	Release,
};

enum class LBlockInputTypes_t : uint8_t {
	Automatic,
	Always,
	Never,
};

//-------------------------------------------------------------------------------------------------------------------
//  BaseEventData_t Struct ----- Defines the type of data each toml table array entry will contain
//-------------------------------------------------------------------------------------------------------------------

struct BaseEventData_t {
	std::string Event;                      //Event Name
	std::vector<std::string> Keys;          //List Of Dinput key names Minus "DIK_ preffix"
	bool Exclusive = false;                 //Exclusive Flag
	std::string Trigger = "Once";           //Trigger Type
	float Duration = 0.0;                   //Trigger Duration
	std::string BlockInput = "Automatic";   //Block Game Input
	bool Disabled = false;                  //Completely Ignore Event
};
TOML_SERIALIZABLE(BaseEventData_t);

//-------------------------------------------------------------------------------------------------------------------
//  InputEvent_t Struct ----- Wrapper around the base input event.
//  Can hold static data that does not need to be serialized/saved.
//-------------------------------------------------------------------------------------------------------------------

struct InputEvent_t {

	const LInputCategory_t UICategory;
	const bool AdvFeature; //Disables/Hides Keybind if Advanced Settings is Disabled
	const char* const UIName = nullptr;
	const char* const UIDescription = nullptr;

	BaseEventData_t Event;
};

namespace Strings::Keybinds {
	constexpr PSString CategoryNames[] = {
			"默认",
			"菜单 / UI",
			"吞噬",
			"拥抱",
			"大腿",
			"抓取",
			"（抓取）胸部",
			"（抓取）玩弄",
			"踩踏",
			"踢击 / 挥击",
			"移动",
			"碾压",
			"（抓取）乳沟",
			"镜头",
			"能力 / Perk",
			"其他"
	};

	static_assert(std::size(CategoryNames) == static_cast<size_t>(LInputCategory_t::kTotal));

}

namespace GTS {

    //----------------------------------------
    // EVENT LIST ----
    //----------------------------------------

    static inline const std::vector<InputEvent_t> DefaultEvents = {
    	
        /* // ----- EXAMPLE
        {
           .UICategory    = LInputCategory_t::kDefault,
		   .AdvFeature    = false,
		   .UIName        = nullptr,
		   .UIDescription = nullptr,
	        {
	            .Event      = "Event",
	            .Keys       = {"A", "B"},
	            .Exclusive  = false,
	            .Duration   = 1.0f,
	            .Trigger    = "Once",
	            .BlockInput = "Automatic",
	            .Disabled   = false,
	        }
        },
        */

		/*Note
		 * If the UI fields are left empty "" or nullptr
		 * the event name is used instead and no description will be displayed on hover.
		 */



        //=======================================================
	    //================ M E N U
	    //=======================================================

	    {
	        .UICategory    = LInputCategory_t::kMenu,
		    .AdvFeature    = false,
		    .UIName        = "设置菜单",
		    .UIDescription = "打开此窗口。也可以禁用此按键绑定，并改用控制台命令 \"gts menu\" 再次打开。",
            .Event = {
		        .Event      = "OpenModSettings",
		        .Keys       = {"F1"},
		        .Trigger    = "Once",
		        .BlockInput = "Always"
		    }
	    },
        {
	        .UICategory    = LInputCategory_t::kMenu,
		    .AdvFeature    = false,
		    .UIName        = "技能树",
		    .UIDescription = "如果已安装 \"Custom Skills Framework\"，则打开技能树。",
			.Event = {
		        .Event      = "OpenSkillTree",
		        .Keys       = {"F4"},
		        .Trigger    = "Once",
		        .BlockInput = "Always"
		    }
        },
		{
	        .UICategory    = LInputCategory_t::kMenu,
		    .AdvFeature    = true,
		    .UIName        = "调试菜单",
		    .UIDescription = "打开调试菜单。",
			.Event = {
				.Event      = "OpenDebugMenu",
				.Keys       = {"LCONTROL", "LSHIFT", "F12"},
				.Trigger    = "Once",
				.BlockInput = "Always"
			}
		},
        {
	        .UICategory    = LInputCategory_t::kMenu,
		    .AdvFeature    = false,
		    .UIName        = "快速状态",
		    .UIDescription = "显示玩家和当前追随者的简要体型信息。",
			.Event = {
                .Event      = "ShowQuickStats",
                .Keys       = {"F3"},
                .BlockInput = "Never"
            }
        },
        {
	        .UICategory    = LInputCategory_t::kMenu,
		    .AdvFeature    = true,
		    .UIName        = "体型报告",
		    .UIDescription = "在控制台打印 NPC/玩家调试信息。",
			.Event = {
                .Event      = "DebugReport",
                .Keys       = {"RCONTROL"},
                .Duration   = 1.33f,
                .BlockInput = "Never"
            }
        },

        //========================================================
	    //================ V O R E
	    //========================================================

		{
	        .UICategory    = LInputCategory_t::kVore,
		    .AdvFeature    = false,
		    .UIName        = "开始吞噬",
		    .UIDescription = "开始基础吞噬动作。",
			.Event = {
		        .Event      = "Vore",
		        .Keys       = {"LSHIFT", "V"},
		        .Trigger    = "Once"
		    }
        },
        {
	        .UICategory    = LInputCategory_t::kVore,
		    .AdvFeature    = false,
		    .UIName        = "开始吞噬（玩家）",
		    .UIDescription = "以玩家为目标开始基础吞噬动作。",
			.Event = {
		        .Event      = "PlayerVore",
		        .Keys       = {"LSHIFT", "V"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.0,
		        .BlockInput = "Never"
		    }

        },

        //========================================================
	    //================ S T O M P S
	    //========================================================

		{
	        .UICategory    = LInputCategory_t::kStomp,
		    .AdvFeature    = false,
		    .UIName        = "开始轻踩（右）",
		    .UIDescription = "开始轻踩动作。",
			.Event = {
		        .Event      = "RightStomp",
		        .Keys       = {"LSHIFT", "E"},
		        .Trigger    = "Release",
		    }
        },
        {
	        .UICategory    = LInputCategory_t::kStomp,
		    .AdvFeature    = false,
		    .UIName        = "开始轻踩（左）",
		    .UIDescription = "开始轻踩动作。",
			.Event = {
		        .Event      = "LeftStomp",
		        .Keys       = {"LSHIFT", "Q"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kStomp,
			.AdvFeature    = false,
			.UIName        = "开始重踩（右）",
			.UIDescription = "开始重踩动作。",
			.Event = {
		        .Event      = "RightStomp_Strong",
		        .Keys       = {"LSHIFT", "E"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.44f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kStomp,
			.AdvFeature    = false,
			.UIName        = "开始重踩（左）",
			.UIDescription = "开始重踩动作。",
			.Event = {
		        .Event      = "LeftStomp_Strong",
		        .Keys       = {"LSHIFT", "Q"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.44f,
		    }
        },
		{
			.UICategory    = LInputCategory_t::kStomp,
			.AdvFeature    = false,
			.UIName        = "开始践踏（右）",
			.UIDescription = "开始践踏动作。",
			.Event = {
		        .Event      = "TrampleRight",
		        .Keys       = {"LSHIFT", "E"},
		        .Trigger    = "Release",
		        .Duration   = 0.20f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kStomp,
			.AdvFeature    = false,
			.UIName        = "开始践踏（左）",
			.UIDescription = "开始践踏动作。",
			.Event = {
		        .Event      = "TrampleLeft",
		        .Keys       = {"LSHIFT", "Q"},
		        .Trigger    = "Release",
		        .Duration   = 0.20f,
		    }
        },

        //========================================================
	    //================ T H I G H  C R U S H
	    //========================================================


        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "开始大腿碾压",
			.UIDescription = "开始大腿碾压动作。",
			.Event = {
		        .Event      = "ThighCrush",
		        .Keys       = {"LSHIFT", "W", "C"},
		        .Exclusive  = true
		    }
        },
        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "大腿碾压攻击",
			.UIDescription = "执行大腿碾压攻击/击杀动作。",
			.Event = {
		        .Event      = "ThighCrushKill",
		        .Keys       = {"LMB"},
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "退出大腿碾压",
			.UIDescription = "退出大腿碾压模式。",
			.Event = {
		        .Event      = "ThighCrushSpare",
		        .Keys       = {"W"},
		        .Exclusive  = true,
		        .BlockInput = "Never"
		    }
        },

		//========================================================
		//============ T H I G H  S A N D W I C H
		//========================================================

        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "开始大腿夹击",
			.UIDescription = "开始大腿夹击动作。",
			.Event = {
		        .Event      = "ThighSandwichEnter",
		        .Keys       = {"LSHIFT", "C"},
		        .Exclusive  = true,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "开始大腿夹击（玩家）",
			.UIDescription = "以玩家为目标开始大腿夹击动作。",
			.Event = {
		        .Event      = "PlayerThighSandwichEnter",
		        .Keys       = {"LSHIFT", "C"},
		        .Exclusive  = true,
		        .Trigger    = "Continuous",
		        .Duration   = 1.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "大腿夹击重攻击",
			.UIDescription = "在大腿夹击模式中执行重攻击。",
			.Event = {
		        .Event      = "ThighSandwichAttackHeavy",
		        .Keys       = {"LMB"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "大腿夹击轻攻击",
			.UIDescription = "在大腿夹击模式中执行轻攻击。",
			.Event = {
		        .Event      = "ThighSandwichAttack",
		        .Keys       = {"LMB"},
		        .Trigger    = "Release",
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kThighs,
			.AdvFeature    = false,
			.UIName        = "退出大腿夹击",
			.UIDescription = "退出大腿夹击模式。",
			.Event = {
		        .Event      = "ThighSandwichExit",
		        .Keys       = {"W"},
		        .Exclusive  = true,
		        .BlockInput = "Never"
		    }
        },


		//========================================================
		//============ T H I G H  S A N D W I C H - P A R T  2
		//========================================================

		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "进入臀部大腿夹击",
			.UIDescription = "进入臀部大腿夹击模式。",
			.Event = {
				.Event = "SandwichButtStart",
				.Keys = {"LSHIFT", "H"},
				.Exclusive = true,
			}
		},
		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "退出臀部大腿夹击",
			.UIDescription = "退出臀部大腿夹击模式。",
			.Event = {
				.Event = "SandwichButtStop",
				.Keys = {"LSHIFT", "H"},
				.Exclusive = true,
			}
		},
		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "臀部大腿夹击重攻击",
			.UIDescription = "在臀部大腿夹击模式中执行重臀部攻击。",
			.Event = {
				.Event = "SandwichHeavyAttack",
				.Keys       = {"LMB"},
		        .Exclusive  = true,
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		        .BlockInput = "Never"
			}
		},
		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "臀部大腿夹击轻攻击",
			.UIDescription = "在臀部大腿夹击模式中执行轻臀部攻击。",
			.Event = {
				.Event = "SandwichLightAttack",
				.Keys = {"LMB"},
				.Exclusive = true,
				.Trigger = "Release"
			}
		},
		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "大腿夹击成长",
			.UIDescription = "在臀部大腿夹击模式中成长。需要 Perk。",
			.Event = {
				.Event = "SandwichGrowth",
				.Keys = {"W"},
				.Exclusive = true
			}
		},
		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "开始大腿夹击碾磨",
			.UIDescription = "对被臀部夹住的 NPC 执行持续碾磨动作。",
			.Event = {
				.Event = "SandwichGrindStart",
				.Keys = {"S"},
				.Exclusive = true,
			}
		},
		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "停止大腿夹击碾磨",
			.UIDescription = "停止碾磨被臀部夹住的 NPC。",
			.Event = {
				.Event = "SandwichGrindStop",
				.Keys = {"S"},
				.Exclusive = true,
			}
		},

		{
			.UICategory = LInputCategory_t::kThighs,
			.AdvFeature = false,
			.UIName = "大腿夹击逆产/吞噬",
			.UIDescription = "吞噬被臀部夹住的 NPC。",
			.Event = {
				.Event = "SandwichUB",
				.Keys = {"V"},
				.Exclusive = true,
				.Duration = 0.33f,
			}
		},

        //========================================================
	    //============ K I C K S
	    //========================================================

        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始轻踢（右）",
			.UIDescription = "执行轻踢。",
			.Event = {
		        .Event      = "LightKickRight",
		        .Keys       = {"LALT", "E"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始轻踢（左）",
			.UIDescription = "执行轻踢。",
			.Event = {
		        .Event      = "LightKickLeft",
		        .Keys       = {"LALT", "Q"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始重踢（右）",
			.UIDescription = "执行重踢。",
			.Event = {
		        .Event      = "HeavyKickRight",
		        .Keys       = {"LALT", "E"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始重踢（左）",
			.UIDescription = "执行重踢。",
			.Event = {
		        .Event      = "HeavyKickLeft",
		        .Keys       = {"LALT", "Q"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始重低踢（右）",
			.UIDescription = "执行重踢的替代版本。",
			.Event = {
		        .Event      = "HeavyKickRight_Low",
		        .Keys       = {"LALT", "E"},
		        .Trigger    = "Release",
		        .Duration   = 0.1f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始重低踢（左）",
			.UIDescription = "执行重踢的替代版本。",
			.Event = {
		        .Event      = "HeavyKickLeft_Low",
		        .Keys       = {"LALT", "Q"},
		        .Trigger    = "Release",
		        .Duration   = 0.1f,
		    }
        },

        //========================================================
	    //============ C R A W L I N G
	    //========================================================

        {
			.UICategory    = LInputCategory_t::kMovement,
			.AdvFeature    = false,
			.UIName        = "切换爬行（玩家）",
			.UIDescription = "在常规潜行动画和替代爬行动画之间切换。",
			.Event = {
		        .Event      = "TogglePlayerCrawl",
		        .Keys       = {"NUMPAD1"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kMovement,
			.AdvFeature    = false,
			.UIName        = "切换爬行（追随者）",
			.UIDescription = "在常规潜行动画和替代爬行动画之间切换。",
			.Event = {
		        .Event      = "ToggleFollowerCrawl",
		        .Keys       = {"NUMPAD3"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始轻挥击（右）",
			.UIDescription = "执行轻挥击攻击（仅在潜行/爬行时生效）。",
			.Event = {
		        .Event      = "LightSwipeRight",
		        .Keys       = {"LALT", "E"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始轻挥击（左）",
			.UIDescription = "执行轻挥击攻击（仅在潜行/爬行时生效）。",
			.Event = {
		        .Event      = "LightSwipeLeft",
		        .Keys       = {"LALT", "Q"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始重挥击（右）",
			.UIDescription = "执行重挥击攻击（仅在潜行/爬行时生效）。",
			.Event = {
		        .Event      = "HeavySwipeRight",
		        .Keys       = {"LALT", "E"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kKickSwipe,
			.AdvFeature    = false,
			.UIName        = "开始重挥击（左）",
			.UIDescription = "执行重挥击攻击（仅在潜行/爬行时生效）。",
			.Event = {
		        .Event      = "HeavySwipeLeft",
		        .Keys       = {"LALT", "Q"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		    }
        },

        //========================================================
	    //============ C L E A V A G E
	    //========================================================

        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "开始乳沟模式",
			.UIDescription = "进入乳沟模式（需要先把 NPC 放进去）。",
			.Event = {
		        .Event      = "CleavageEnter",
		        .Keys       = {"LSHIFT", "H"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "退出乳沟模式",
			.UIDescription = "退出乳沟模式并返回胸部抓握模式。",
			.Event = {
		        .Event      = "CleavageExit",
		        .Keys       = {"RMB"},
		        .BlockInput = "Never"
		    }
        },
		{
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "乳沟轻攻击",
			.UIDescription = "执行轻攻击。",
			.Event = {
		        .Event      = "CleavageLightAttack",
		        .Keys       = {"LMB"},
		        .Exclusive  = true,
		        .Trigger    = "Release",
		        .BlockInput = "Never"
		    }
		},
        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "乳沟重攻击",
			.UIDescription = "执行重攻击。",
			.Event = {
		        .Event      = "CleavageHeavyAttack",
		        .Keys       = {"LMB"},
		        .Exclusive  = true,
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "乳沟窒息",
			.UIDescription = "执行乳沟窒息攻击/动作。",
			.Event = {
		        .Event      = "CleavageSuffocate",
		        .Keys       = {"W"},
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "乳沟吸收",
			.UIDescription = "吸收（并杀死）被夹住的 NPC。",
			.Event = {
		        .Event      = "CleavageAbsorb",
		        .Keys       = {"S"},
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "乳沟吞噬",
			.UIDescription = "吞噬/吃掉被夹住的 NPC。",
			.Event = {
		        .Event      = "CleavageVore",
		        .Keys       = {"V"},
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCleavage,
			.AdvFeature    = false,
			.UIName        = "乳沟勒杀",
			.UIDescription = "执行乳沟勒杀攻击/动作。",
			.Event = {
		        .Event      = "CleavageDOT",
		        .Keys       = {"E"},
		    }
        },

        //========================================================
		//============ H U G S
		//========================================================

        {
			.UICategory    = LInputCategory_t::kHugs,
			.AdvFeature    = false,
			.UIName        = "开始拥抱",
			.UIDescription = "开始拥抱动作并进入拥抱模式。",
			.Event = {
		        .Event      = "HugAttempt",
		        .Keys       = {"LSHIFT", "H"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kHugs,
			.AdvFeature    = false,
			.UIName        = "开始拥抱（玩家）",
			.UIDescription = "以玩家为目标开始拥抱动作并进入拥抱模式。",
			.Event = {
		        .Event      = "HugPlayer",
		        .Keys       = {"LSHIFT", "H"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kHugs,
			.AdvFeature    = false,
			.UIName        = "拥抱缩小",
			.UIDescription = "执行拥抱缩小攻击/动作。",
			.Event = {
		        .Event      = "HugShrink",
		        .Keys       = {"LMB"},
		        .Exclusive  = true,
		        .Trigger    = "Release"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kHugs,
			.AdvFeature    = false,
			.UIName        = "拥抱治疗",
			.UIDescription = "执行拥抱治疗动作。",
			.Event = {
		        .Event      = "HugHeal",
		        .Keys       = {"LMB"},
		        .Exclusive  = true,
		        .Trigger    = "Continuous",
		        .Duration   = 0.33f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kHugs,
			.AdvFeature    = false,
			.UIName        = "拥抱碾压",
			.UIDescription = "执行拥抱碾压动作（会杀死被抱住的目标）。",
			.Event = {
		        .Event      = "HugCrush",
		        .Keys       = {"S"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kHugs,
			.AdvFeature    = false,
			.UIName        = "退出/释放拥抱",
			.UIDescription = "释放被抱住的 NPC/目标并退出拥抱模式。",
			.Event = {
		        .Event      = "HugRelease",
		        .Keys       = {"RMB"},
		        .Exclusive  = true,
		        .BlockInput = "Never"
			}
        },

        //========================================================
	    //============ B U T T / B R E A S T   C R U S H
	    //========================================================

        {
			.UICategory    = LInputCategory_t::kCrush,
			.AdvFeature    = false,
			.UIName        = "开始快速碾压",
			.UIDescription = "开始无目标臀部/胸部碾压动作。",
			.Event = {
		        .Event      = "QuickButtCrushStart",
		        .Keys       = {"LSHIFT", "B"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.0f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCrush,
			.AdvFeature    = false,
			.UIName        = "开始锁定目标碾压",
			.UIDescription = "开始锁定目标的臀部/胸部碾压动作并进入碾压模式。",
			.Event = {
		        .Event      = "ButtCrushStart",
		        .Keys       = {"LSHIFT", "B"},
		        .Trigger    = "Release",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCrush,
			.AdvFeature    = false,
			.UIName        = "开始锁定目标碾压（玩家）",
			.UIDescription = "以玩家为目标开始锁定目标的臀部/胸部碾压动作并进入碾压模式。",
			.Event = {
		        .Event      = "ButtCrushStart_Player",
		        .Keys       = {"LSHIFT", "B"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCrush,
			.AdvFeature    = false,
			.UIName        = "碾压成长",
			.UIDescription = "在碾压模式中执行成长动作。",
			.Event = {
		        .Event      = "ButtCrushGrow",
		        .Keys       = {"W"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCrush,
			.AdvFeature    = false,
			.UIName        = "碾压攻击",
			.UIDescription = "执行碾压攻击，并同时退出碾压模式。",
			.Event = {
		        .Event      = "ButtCrushAttack",
		        .Keys       = {"LMB"},
		        .Duration   = 0.0f,
		    }
        },

        //========================================================
        //======== P R O N E   B E H A V I O R
        //========================================================

        {
			.UICategory    = LInputCategory_t::kMovement,
			.AdvFeature    = false,
			.UIName        = "进入趴伏",
			.UIDescription = "进入趴伏模式（仅在已经潜行/爬行时可用）。",
			.Event = {
		        .Event      = "SBO_ToggleProne",
		        .Keys       = {"X"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.66f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kMovement,
			.AdvFeature    = false,
			.UIName        = "趴伏扑倒（站立）",
			.UIDescription = "未潜行/爬行时通过扑向地面进入趴伏模式。",
			.Event = {
		        .Event      = "SBO_ToggleDive_Standing",
		        .Keys       = {"W", "S"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.50f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kMovement,
			.AdvFeature    = false,
			.UIName        = "趴伏扑倒（潜行）",
			.UIDescription = "潜行/爬行时通过扑向地面进入趴伏模式。",
			.Event = {
		        .Event      = "SBO_ToggleDive_Sneak",
		        .Keys       = {"W", "S"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.50f,
		        .BlockInput = "Never"
		    }
        },

        //========================================================
	    //========================= G R A B
	    //======================================================== 

        {
			.UICategory    = LInputCategory_t::kGrab,
			.AdvFeature    = false,
			.UIName        = "开始抓取",
			.UIDescription = "开始抓取动作并进入抓取模式。",
			.Event = {
		        .Event      = "GrabOther",
		        .Keys       = {"F"},
		        .Duration   = 0.25f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrab,
			.AdvFeature    = false,
			.UIName        = "开始抓取（玩家）",
			.UIDescription = "以玩家为目标开始抓取动作并进入抓取模式。",
			.Event = {
		        .Event      = "GrabPlayer",
		        .Keys       = {"F"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrab,
			.AdvFeature    = false,
			.UIName        = "抓取攻击",
			.UIDescription = "对被抓住的角色执行抓取碾压攻击。",
			.Event = {
				.Event      = "GrabAttack",
		        .Keys       = {"E"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.50f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrab,
			.AdvFeature    = false,
			.UIName        = "抓取吞噬",
			.UIDescription = "吞噬被抓住的角色。",
			.Event = {
		        .Event      = "GrabVore",
		        .Keys       = {"V"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.50f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrab,
			.AdvFeature    = false,
			.UIName        = "抓取投掷",
			.UIDescription = "扔出被抓住的角色并退出抓取模式。",
			.Event = {
		        .Event      = "GrabThrow",
		        .Keys       = {"X"},
		        .Trigger    = "Continuous",
		        .BlockInput = "Never"
		    }
        },
        {
		    .UICategory    = LInputCategory_t::kGrab,
			.AdvFeature    = false,
			.UIName        = "退出/释放抓取",
			.UIDescription = "放下被抓住的角色并退出抓取模式。",
			.Event = {
		        .Event      = "GrabRelease",
		        .Keys       = {"RMB"},
		    }
        },

		//========================================================
		//========================= G R A B  B R E A S T S
		//======================================================== 


        {
			.UICategory    = LInputCategory_t::kBreats,
			.AdvFeature    = false,
			.UIName        = "放入胸部",
			.UIDescription = "把被抓住的角色放到持有者胸间。\n（这会离开抓取模式，因此可在胸间已有 NPC 时再抓另一个 NPC。）",
			.Event = {
		        .Event      = "BreastsPut",
		        .Keys       = {"LSHIFT", "B"},
		        .Duration   = 0.50f,
		    }
        },
        {
			.UICategory    = LInputCategory_t::kBreats,
			.AdvFeature    = false,
			.UIName        = "移出胸部",
			.UIDescription = "将已放置的角色从持有者胸间移出。",
			.Event = {
		        .Event      = "BreastsRemove",
		        .Keys       = {"LSHIFT", "B"},
		        .Duration   = 0.50f,
		    }
        },


        //========================================================
	    //========================= G R A B  P L A Y
	    //======================================================== 

        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "开始抓取玩弄",
			.UIDescription = "与被抓住的角色进入抓取玩弄模式。",
			.Event = {
		        .Event      = "GrabPlay_Start",
		        .Keys       = {"LSHIFT", "H"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "退出抓取玩弄",
			.UIDescription = "退出抓取玩弄模式。",
			.Event = {
		        .Event      = "GrabPlay_Exit",
		        .Keys       = {"LSHIFT","H"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄碾压",
			.UIDescription = "执行碾压攻击。",
			.Event = {
		        .Event      = "GrabPlay_CrushHeavy",
		        .Keys       = {"LMB"},
		        .Trigger    = "Continuous",
		        .Duration   = 0.5f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄吞噬",
			.UIDescription = "吞噬被抓住的角色。",
			.Event = {
		        .Event      = "GrabPlay_Vore",
		        .Keys       = {"V"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄亲吻",
			.UIDescription = "亲吻被抓住的角色并治疗他们。",
			.Event = {
		        .Event      = "GrabPlay_Kiss",
		        .Keys       = {"E"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
		{
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "亲吻时抓取吞噬",
			.UIDescription = "执行亲吻动作时吞噬被抓住的角色。",
			.Event = {
				.Event      = "GrabPlay_KissVore",
				.Keys       = {"V"},
				.Trigger    = "Once",
				.Duration   = 0.0f,
				.BlockInput = "Never"
			}
		},
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄戳弄",
			.UIDescription = "戳弄被抓住的角色。",
			.Event = {
		        .Event      = "GrabPlay_Poke",
		        .Keys       = {"LMB"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄弹开",
			.UIDescription = "将被抓住的角色弹飞，并退出抓取模式。",
			.Event = {
		        .Event      = "GrabPlay_Flick",
		        .Keys       = {"RMB"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄夹击",
			.UIDescription = "执行夹击动作。",
			.Event = {
		        .Event      = "GrabPlay_Sandwich",
		        .Keys       = {"S"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄碾磨（进入）",
			.UIDescription = "进入抓取玩弄碾磨模式，会缓慢伤害被抓住的角色。",
			.Event = {
		        .Event      = "GrabPlay_GrindStart",
		        .Keys       = {"W"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kGrabPlay,
			.AdvFeature    = false,
			.UIName        = "抓取玩弄碾磨（退出）",
			.UIDescription = "退出抓取玩弄碾磨模式。",
			.Event = {
		        .Event      = "GrabPlay_GrindStop",
		        .Keys       = {"W"},
		        .Trigger    = "Once",
		        .Duration   = 0.0f,
		        .BlockInput = "Never"
		    }
        },

        //========================================================
	    //========================= C A M E R A
	    //========================================================

        {
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "镜头重置（水平）",
			.UIDescription = "重置临时水平镜头偏移。",
			.Event = {
		        .Event      = "HorizontalCameraReset",
		        .Keys       = {"RIGHT", "LEFT"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "镜头重置（垂直）",
			.UIDescription = "重置临时垂直镜头偏移。",
			.Event = {
		        .Event      = "VerticalCameraReset",
		        .Keys       = {"UP", "DOWN"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "移动镜头（右）",
			.UIDescription = "临时将镜头向右移动。",
			.Event = {
		        .Event      = "CameraRight",
		        .Keys       = {"LALT", "RIGHT"},
		        .Trigger    = "Continuous",
		    }
        },
		{
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "移动镜头（左）",
			.UIDescription = "临时将镜头向左移动。",
			.Event = {
				.Event      = "CameraLeft",
				.Keys       = {"LALT", "LEFT"},
				.Trigger    = "Continuous",
			}
		},
        {
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "移动镜头（上）",
			.UIDescription = "临时将镜头向上移动。",
			.Event = {
		        .Event      = "CameraUp",
		        .Keys       = {"LALT", "UP"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "移动镜头（下）",
			.UIDescription = "临时将镜头向下移动。",
			.Event = {
		        .Event      = "CameraDown",
		        .Keys       = {"LALT", "DOWN"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kCamera,
			.AdvFeature    = false,
			.UIName        = "循环镜头模式",
			.UIDescription = "在替代镜头模式之间切换。",
			.Event = {
		        .Event      = "SwitchCameraMode",
		        .Keys       = {"F2"}
		    }
        },

        //========================================================
		//========================= A N I M  S P E E D
		//========================================================

        {
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "攻击加速",
			.UIDescription = "提高部分动画的攻击速度。",
			.Event = {
		        .Event      = "AnimSpeedUp",
		        .Keys       = {"LMB"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "攻击减速",
			.UIDescription = "降低部分动画的攻击速度。",
			.Event = {
		        .Event      = "AnimSpeedDown",
		        .Keys       = {"RMB"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "攻击最高速度",
			.UIDescription = "将支持动作的攻击速度设为最高。",
			.Event = {
		        .Event      = "AnimMaxSpeed",
		        .Keys       = {"LMB", "RMB"},
		        .Trigger    = "Continuous",
		    }
        },
		{
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "挣扎向上",
			.UIDescription = "尝试挣脱拥抱或抓取。",
			.Event = {
		        .Event      = "StruggleUp",
		        .Keys       = {"W"},
		        .Trigger    = "Once",
		    }
        },
		{
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "挣扎向下",
			.UIDescription = "尝试挣脱拥抱或抓取。",
			.Event = {
		        .Event      = "StruggleDown",
		        .Keys       = {"S"},
		        .Trigger    = "Once",
		    }
        },
		{
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "挣扎向左",
			.UIDescription = "尝试挣脱拥抱或抓取。",
			.Event = {
		        .Event      = "StruggleLeft",
		        .Keys       = {"A"},
		        .Trigger    = "Once",
		    }
        },
		{
			.UICategory    = LInputCategory_t::kMisc,
			.AdvFeature    = false,
			.UIName        = "挣扎向右",
			.UIDescription = "尝试挣脱拥抱或抓取。",
			.Event = {
		        .Event      = "StruggleRight",
		        .Keys       = {"D"},
		        .Trigger    = "Once",
		    }
        },

        //========================================================
	    //========================= A B I L I T I E S
	    //========================================================

        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "缩小爆发",
			.UIDescription = "执行缩小爆发攻击，缩小你附近的所有人。",
			.Event = {
		        .Event      = "ShrinkOutburst",
		        .Keys       = {"LSHIFT", "F"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "体型储备（使用）",
			.UIDescription = "应用已储存的体型储备（按住越久，转换的体型越多）。",
			.Event = {
		        .Event      = "SizeReserve",
		        .Keys       = {"E"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.33f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "体型储备（显示）",
			.UIDescription = "显示包含当前储存体型储备量的通知。",
			.Event = {
		        .Event      = "DisplaySizeReserve",
		        .Keys       = {"F"},
		        .Trigger    = "Continuous",
		        .Duration   = 1.33f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "成长（快速）",
			.UIDescription = "一次性的动画成长爆发。",
			.Event = {
		        .Event      = "RapidGrowth",
		        .Keys       = {"LSHIFT", "1"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "缩小（快速）",
			.UIDescription = "一次性的动画缩小爆发。",
			.Event = {
		        .Event      = "RapidShrink",
		        .Keys       = {"LSHIFT", "2"},
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "成长（手动）",
			.UIDescription = "缓慢持续成长。",
			.Event = {
		        .Event      = "ManualGrow",
		        .Keys       = {"UP", "LEFT"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "缩小（手动）",
			.UIDescription = "缓慢持续缩小。",
			.Event = {
		        .Event      = "ManualShrink",
		        .Keys       = {"DOWN", "LEFT"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "追随者成长（手动）",
			.UIDescription = "缓慢持续成长。",
			.Event = {
		        .Event      = "ManualGrowOther",
		        .Keys       = {"LSHIFT", "UP", "LEFT"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "追随者缩小（手动）",
			.UIDescription = "缓慢持续缩小。",
			.Event = {
		        .Event      = "ManualShrinkOther",
		        .Keys       = {"LSHIFT", "DOWN", "LEFT"},
		        .Trigger    = "Continuous",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "成长（增量）",
			.UIDescription = "一次非动画的增量成长爆发。",
			.Event = {
		        .Event      = "ManualGrowOverTime",
		        .Keys       = {"NUMPAD8"},
		        .Trigger    = "Once",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "缩小（增量）",
			.UIDescription = "一次非动画的增量缩小爆发。",
			.Event = {
		        .Event      = "ManualShrinkOverTime",
		        .Keys       = {"NUMPAD4"},
		        .Trigger    = "Once",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "追随者成长（增量）",
			.UIDescription = "一次非动画的增量成长爆发。",
			.Event = {
		        .Event      = "ManualGrowOtherOverTime",
		        .Keys       = {"NUMPAD5"},
		        .Trigger    = "Once",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "追随者缩小（增量）",
			.UIDescription = "一次非动画的增量缩小爆发。",
			.Event = {
		        .Event      = "ManualShrinkOtherOverTime",
		        .Keys       = {"NUMPAD2"},
		        .Trigger    = "Once",
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "保护小家伙",
			.UIDescription = "激活“保护小家伙”Perk，使你周围的其他人免疫体型伤害。",
			.Event = {
		        .Event      = "ProtectSmallOnes",
		        .Keys       = {"C"},
		        .Duration   = 1.0f,
		        .BlockInput = "Never"
		    }
        },
        {
			.UICategory    = LInputCategory_t::kAbility,
			.AdvFeature    = false,
			.UIName        = "召回缩小目标",
			.UIDescription = "把搜索范围内、已经缩小过的角色召回到你附近，并让他们短暂停步。",
			.Event = {
		        .Event      = "RecallShrunkenActors",
		        .Keys       = {"NUMPAD0"},
		        .Trigger    = "Once",
		        .BlockInput = "Never"
		    }
        }
		
    };
}
