#include "UI/Windows/Settings/Categories/AutoAim.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/ConditionalHeader.hpp"

#include "Managers/MaxSizeManager.hpp"
#include "Config/Config.hpp"

#include "UI/Controls/Text.hpp"


namespace GTS {

	CategoryAutoAim::CategoryAutoAim() {
		m_name = "自动瞄准";
	}

    void CategoryAutoAim::DrawLeft() {

        ImUtil_Unique
		{

            PSString T0 = "启用或禁用部分尺寸动作的自动瞄准。\n"
            "关闭后，玩家会回到旧逻辑：通过低头角度触发脚下动作。\n"
            "该开关只影响玩家；NPC 仍会使用自动瞄准。";
            PSString T1 = "未找到目标时，阻止动作退回到远距离踩踏。";

            PSString THelp = "自动瞄准会让尺寸动作自动选择更合适的目标与左右侧，并减少需要绑定的按键数量。\n"
                    "它适用于轻踩、重踩、践踏、手部拍击等基础尺寸动作。\n"
                    "- 系统会先搜索最近的敌人。\n"
                    "- 然后判断使用哪只手或哪只脚更合适。\n"
                    "- 如果没有找到敌人，则执行随机侧的动作。\n\n"

                    "技术限制：\n"
                    "- Skyrim 没有完整可用的脚部 IK 系统。\n"
                    "- 因此这里通过混合多段动画来近似瞄准目标。\n"
                    "- 偶尔可能出现不完美的姿态，但通常比固定动画更可靠。\n"
                    "- 混合量由玩家与目标之间的角度和距离决定。\n"
                    "- 内部会驱动 GTS_StompBlend、GTS_StompBlend_X 和 GTS_StompBlend_Y 动画变量。\n"
                    "- 这些变量最终决定实际播放的动作。";
            if (ImGui::CollapsingHeader("自动瞄准", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是自动瞄准", THelp);
                ImGuiEx::CheckBox("启用自动瞄准", &Config::AutoAim.bEnableAutoAim, T0);
                ImGuiEx::CheckBox("阻止远距离踩踏", &Config::AutoAim.bPreventFarStomps, T1);

                ImGui::Spacing();
            }
        }

        ImUtil_Unique
		{
            PSString THelp =
            "- 深绿色：搜索原点\n"
            "- 粉色：臀部 / 胸部动作\n"
            "- 红色：近距离踩踏\n"
            "- 绿色：远距离踩踏\n"
            "- 浅蓝色：踢击\n"
            "- 亮绿色：命中的敌人";
            PSString T0 = "显示自动瞄准的搜索范围和当前目标。";

            if (ImGui::CollapsingHeader("调试", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("调试颜色说明", THelp);
                ImGuiEx::CheckBox("显示自动瞄准范围", &Config::AutoAim.bDebugAutoAim, T0);
                ImGui::Spacing();
            }
        }
        // ---- Action settings and offsets

        ImUtil_Unique {
            PSString THelp = "近距离踩踏会在敌人靠近腿部时触发。\n"
            "适合命中试图躲到脚边或脚下的敌人。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：36.5";
            PSString T1 = "[偏移] 初始搜索碰撞体相对角色左右移动的距离。\n"
            "默认：10.0";
            if (ImGui::CollapsingHeader("近距离踩踏设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是近距离踩踏", THelp);
                ImGuiEx::SliderF("踩踏半径", &Config::AutoAim.fAutoAim_Range_Stomp, 30.0f, 60.0f, T0, "%.2f");
                ImGuiEx::SliderF("左右偏移", &Config::AutoAim.fAutoAim_Foot_OffsetDistance, 0.0f, 30.0f, T1, "%.2f");
                ImGui::Spacing();
            }
        }

        ImUtil_Unique
		{
            PSString THelp = "远距离踩踏会在敌人超出普通踩踏范围时触发。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：56.0";
            PSString T1 = "[半径] 决定重踩版本的远距离搜索碰撞体大小。\n"
            "默认：48.0";
            PSString T2 = "[偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：0.0";

            if (ImGui::CollapsingHeader("远距离踩踏设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是远距离踩踏", THelp);
                ImGuiEx::SliderF("碰撞体大小", &Config::AutoAim.fAutoAim_Range_FarStomp, 10.0f, 90.0f, T0, "%.2f");
                ImGuiEx::SliderF("[重踩] 碰撞体大小", &Config::AutoAim.fAutoAim_Range_FarStomp_Strong, 10.0f, 90.0f, T1, "%.2f");
                ImGuiEx::SliderF("前后偏移", &Config::AutoAim.fAutoAim_Foot_OffsetDistance_FarStomp, 0.0f, 75.0f, T2, "%.2f");
                ImGui::Spacing();
            }
        }

        ImUtil_Unique {
            PSString THelp = "手部拍击是潜行状态下的轻踩变体。\n"
            "角色会用手拍击地面。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：15.0";
            PSString T1 = "[偏移] 初始搜索碰撞体相对角色左右移动的距离。\n"
            "默认：14.5";
            PSString T2 = "[偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：50.0";
            if (ImGui::CollapsingHeader("[潜行] 手部拍击设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是手部拍击", THelp);
                ImGuiEx::SliderF("手部拍击半径", &Config::AutoAim.fAutoAim_Range_Hand, 10.0f, 30.0f, T0, "%.2f");
                ImGuiEx::SliderF("左右偏移", &Config::AutoAim.fAutoAim_Hand_OffsetDistance_Side, 5.0f, 60.0f, T1, "%.2f");
                ImGuiEx::SliderF("前后偏移", &Config::AutoAim.fAutoAim_Hand_OffsetDistance_Forward, 40.0f, 70.0f, T2, "%.2f");
                ImGui::Spacing();
            }
        }
        ImUtil_Unique {
            PSString THelp = "手部拍击是爬行状态下的轻踩变体。\n"
            "角色会用手拍击地面。";
            PSString THelp1 = "强力手部拍击是爬行状态下的重踩变体。\n"
            "角色会用更强的手部动作拍击地面。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：25.0";
            PSString T1 = "[偏移] 初始搜索碰撞体相对角色左右移动的距离。\n"
            "默认：10.0";
            PSString T2 = "[偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：40.0";
            if (ImGui::CollapsingHeader("[爬行] 手部拍击设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是手部拍击", THelp);
                ImGuiEx::SliderF("手部拍击半径", &Config::AutoAim.fAutoAim_Range_Hand_Crawl, 10.0f, 30.0f, T0, "%.2f");
                ImGuiEx::SliderF("左右偏移", &Config::AutoAim.fAutoAim_Hand_Crawl_OffsetDistance_Side, 5.0f, 60.0f, T1, "%.2f");
                ImGuiEx::SliderF("前后偏移", &Config::AutoAim.fAutoAim_Hand_Crawl_OffsetDistance_Forward, 40.0f, 70.0f, T2, "%.2f");
                ImGuiEx::HelpText("什么是强力手部拍击", THelp1);
                ImGui::Spacing();
            }
        }
        ImUtil_Unique {
            PSString THelp = "踢击是一类可以同时命中多个敌人的尺寸动作。\n"
            "[站立] 角色未潜行或爬行时执行踢击。\n"
            "[手部挥击] 角色潜行或爬行时执行挥击。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：36.0";
            PSString T01 = "[半径] 决定潜行 / 爬行状态的搜索碰撞体大小。\n"
            "默认：36.0";
            PSString T1 = "[偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：20.0";
            PSString T2 = "[手部挥击偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：35.0";
            if (ImGui::CollapsingHeader("踢击设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是踢击", THelp);
                ImGuiEx::SliderF("踢击半径", &Config::AutoAim.fAutoAim_Range_Kick, 20.0f, 60.0f, T0, "%.2f");
                ImGuiEx::SliderF("[潜行] 踢击半径", &Config::AutoAim.fAutoAim_Range_Kick_Sneak, 20.0f, 60.0f, T01, "%.2f");
                ImGuiEx::SliderF("[站立] 前后偏移", &Config::AutoAim.fAutoAim_Kick_OffsetDistance_Forward, 10.0f, 50.0f, T1, "%.2f");
                ImGuiEx::SliderF("[潜行] 前后偏移", &Config::AutoAim.fAutoAim_Hand_OffsetDistance_Forward_Sneak, 20.0f, 50.0f, T2, "%.2f");
                ImGui::Spacing();
            }
        }

        ImUtil_Unique {
            PSString THelp = "臀部砸击是潜行状态下的重踩变体。\n"
            "当敌人在角色腿部或臀部下方时会执行。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：40.0";
            PSString T1 = "[偏移] 初始搜索碰撞体相对角色左右移动的距离。\n"
            "默认：15.0";
            PSString T2 = "[偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：0.0";
            if (ImGui::CollapsingHeader("臀部砸击设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是臀部砸击", THelp);
                ImGuiEx::SliderF("半径", &Config::AutoAim.fAutoAim_Range_ButtSlam, 20.0f, 75.0f, T0, "%.2f");
                ImGuiEx::SliderF("左右偏移", &Config::AutoAim.fAutoAim_Butt_OffsetDistance_Side, 5.0f, 25.0f, T1, "%.2f");
                ImGuiEx::SliderF("前后偏移", &Config::AutoAim.fAutoAim_Butt_OffsetDistance_Forward, 0.0f, 30.0f, T2, "%.2f");
                ImGui::Spacing();
            }
        }

        ImUtil_Unique {
            PSString THelp = "胸部砸击是爬行状态下的重踩变体。\n"
            "当敌人在角色身体下方且手部够不到时会执行。";
            PSString T0 = "[半径] 决定搜索碰撞体大小。\n"
            "默认：32.0";
            PSString T1 = "[偏移] 初始搜索碰撞体相对角色左右移动的距离。\n"
            "默认：10.0";
            PSString T2 = "[偏移] 初始搜索碰撞体相对角色前后移动的距离。\n"
            "默认：35.0";
            if (ImGui::CollapsingHeader("胸部砸击设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::HelpText("什么是胸部砸击", THelp);
                ImGuiEx::SliderF("半径", &Config::AutoAim.fAutoAim_Range_BreastSlam, 20.0f, 75.0f, T0, "%.2f");
                ImGuiEx::SliderF("左右偏移", &Config::AutoAim.fAutoAim_Breast_OffsetDistance_Side, 0.0f, 25.0f, T1, "%.2f");
                ImGuiEx::SliderF("前后偏移", &Config::AutoAim.fAutoAim_Breast_OffsetDistance_Forward, 15.0f, 75.0f, T2, "%.2f");
                ImGui::Spacing();
            }
        }
    }

    //Behavior settings
    void CategoryAutoAim::DrawRight() {
        ImUtil_Unique
		{
            PSString T0 = "使用菱形搜索范围，而不是球形范围。\n"
            "这可能让动画混合结果更准确，但会缩短部分自动瞄准的触发距离。";

            if (ImGui::CollapsingHeader("碰撞体形状", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("使用菱形范围", &Config::AutoAim.bUseRhombShape, T0);

                ImGui::Spacing();
            }
        }
        ImUtil_Unique
		{
            PSString T0 ="降低角色背后敌人的目标优先级。\n"
            "数值越高，自动瞄准越不容易选择身后的敌人。\n"
            "默认：10.0";
            PSString T1 = "降低死亡敌人的目标优先级。\n"
            "数值越高，自动瞄准越不容易选择已经死亡的敌人。\n"
            "默认：20.0";
            PSString T2 = "身后的敌人超过搜索范围的这个比例后，会被忽略。\n"
            "默认：0.25";
            PSString T3 = "放大自动瞄准使用的动画混合值。\n"
            "数值越高，搜索范围边缘目标的瞄准修正越明显。\n"
            "默认：1.25";
            PSString T4 = "未找到目标时，控制 X/Y 动画混合值的随机幅度。\n"
            "默认：0.25";

            if (ImGui::CollapsingHeader("自动瞄准行为设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::SliderF("身后目标惩罚", &Config::AutoAim.fAutoAim_BackPenalty, 0.01f, 30.0f, T0, "%.2f");
                ImGuiEx::SliderF("死亡目标惩罚", &Config::AutoAim.fAutoAim_DeadPenalty, 0.01f, 30.0f, T1, "%.2f");
                ImGuiEx::SliderF("忽略身后目标比例", &Config::AutoAim.fAutoAim_IgnoreBehindAfter, 0.0f, 1.0f, T2, "%.2f");
                ImGuiEx::SliderF("自动瞄准混合倍率", &Config::AutoAim.fAutoAim_AimMagnitudeMultiplier, 1.0f, 1.5f, T3, "%.2fx");
                ImGuiEx::SliderF("未命中随机混合", &Config::AutoAim.fAutoAim_NoHitValueRandomRange, 0.0f, 1.0f, T4, "%.2fx");
                ImGui::Spacing();
            }
        }
    }
}
