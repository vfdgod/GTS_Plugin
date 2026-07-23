#include "UI/Windows/Settings/Categories/AI.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/CollapsingTabHeader.hpp"
#include "UI/Controls/Slider.hpp"

#include "Config/Config.hpp"

#include "UI/Controls/Text.hpp"

namespace {

    using namespace GTS;

    void DrawAIAction_Vore() {

        PSString T0 = "允许开始吞噬动作。";
        PSString T1 = "设置开始吞噬动作的概率。";

        ImGuiEx::CheckBox("启用动作", &Config::AI.Vore.bEnableAction, T0);
        ImGuiEx::SliderF("开始概率", &Config::AI.Vore.fProbability, 1.0f, 100.0f, T1, "%.0f%%", !Config::AI.Vore.bEnableAction);
    }

    void DrawAIAction_Stomps() {

        PSString T0 = "站立时启用脚踩踏，潜行/爬行时启用手部踩踏。";
        PSString T1 = "设置开始踩踏动作的概率。";
        PSString T2 = "设置执行脚下踩踏时触发碾磨动画的概率。";

        ImGuiEx::CheckBox("启用动作", &Config::AI.Stomp.bEnableAction, T0);
        ImGuiEx::SliderF("开始概率", &Config::AI.Stomp.fProbability, 1.0f, 100.0f, T1, "%.0f%%",!Config::AI.Stomp.bEnableAction);
        ImGuiEx::SliderF("脚下踩踏碾磨概率", &Config::AI.Stomp.fUnderstompGrindProbability, 0.0f, 100.0f, T2, "%.0f%%", !Config::AI.Stomp.bEnableAction);

    }

    void DrawAIAction_KickSwipe() {

        PSString T0 = "站立时启用踢击，潜行/爬行时启用手部挥击。";
        PSString T1 = "设置开始踢击或挥击动作的概率。";

        ImGuiEx::CheckBox("启用动作", &Config::AI.KickSwipe.bEnableAction, T0);
        ImGuiEx::SliderF("开始概率", &Config::AI.KickSwipe.fProbability, 1.0f, 100.0f, T1,"%.0f%%",!Config::AI.KickSwipe.bEnableAction);

    }

    void DrawAIAction_ThighSandwich() {

        PSString T0 = "启用大腿夹击动作。";
        PSString T1 = "设置开始大腿夹击动作的概率。";

        PSString T2 = "大腿夹击动作开始后，\n"
			          "调整尝试攻击的时间间隔。\n"
			          "是否攻击取决于下方设置的概率。";

        PSString T3 = "调整开始重攻击的概率。";
        PSString T4 = "调整开始轻攻击的概率。";
        PSString T5 = "调整进入臀部碾磨模式的概率。";
        PSString T6 = "调整臀部模式中开始轻攻击的概率。";
        PSString T7 = "调整臀部模式中开始重攻击的概率。";
        PSString T8 = "调整臀部模式中触发成长爆发的概率。";
        PSString T9 = "调整开始臀部碾磨动作的概率。";
        PSString T10 = "调整停止臀部碾磨动作的概率。";
        PSString T11 = "调整离开臀部模式并返回基础大腿夹击状态的概率。";

        ImGuiEx::CheckBox("启用动作", &Config::AI.ThighSandwich.bEnableAction, T0);

        ImGui::BeginDisabled(!Config::AI.ThighSandwich.bEnableAction);
        {
            ImGuiEx::SliderF("开始概率", &Config::AI.ThighSandwich.fProbability, 1.0f, 100.0f, T1, "%.0f%%");
            ImGuiEx::SliderF("动作间隔", &Config::AI.ThighSandwich.fInterval, 1.0f, 5.0f, T2, "每 %.1f 秒");

            ImGui::Spacing();
            ImGui::Text("基础大腿夹击");

            ImGuiEx::SliderF("重攻击概率", &Config::AI.ThighSandwich.fHeavyAttackPob, 0.0f, 100.0f, T3, "%.0f%%");
            ImGuiEx::SliderF("轻攻击概率", &Config::AI.ThighSandwich.fLightAttackProb, 0.0f, 100.0f, T4, "%.0f%%");

            ImGui::Spacing();
            ImGui::Text("臀部模式选项");

            ImGuiEx::SliderF("进入臀部模式概率", &Config::AI.ThighSandwich.fEnterButtModeProb, 0.0f, 100.0f, T5, "%.0f%%");

            ImGuiEx::SliderF("臀部模式轻攻击概率", &Config::AI.ThighSandwich.fButtLAtkProb, 0.0f, 100.0f, T6, "%.0f%%");
            ImGuiEx::SliderF("臀部模式重攻击概率", &Config::AI.ThighSandwich.fButtHAtkProb, 0.0f, 100.0f, T7, "%.0f%%");

            ImGuiEx::SliderF("臀部模式成长概率", &Config::AI.ThighSandwich.fButtGrowProb, 0.0f, 100.0f, T8, "%.0f%%");

            ImGuiEx::SliderF("开始臀部碾磨概率", &Config::AI.ThighSandwich.fButtGrindStart, 0.0f, 100.0f, T9, "%.0f%%");
            ImGuiEx::SliderF("停止臀部碾磨概率", &Config::AI.ThighSandwich.fButtGrindStop, 0.0f, 100.0f, T10, "%.0f%%");

            ImGuiEx::SliderF("退出臀部模式概率", &Config::AI.ThighSandwich.fButtExitProb, 0.0f, 100.0f, T11, "%.0f%%");
        }
        ImGui::EndDisabled();

    }

    void DrawAIAction_ThighCrush() {

        PSString T0 = "启用大腿碾压动作。";
        PSString T1 = "设置开始大腿碾压动作的概率。";

        PSString T2 = "大腿碾压动作开始后，\n"
                      "调整尝试攻击的时间间隔。\n"
                      "是否攻击取决于下方设置的概率。";

        PSString T3 = "调整执行攻击的概率。";

        ImGuiEx::CheckBox("启用动作", &Config::AI.ThighCrush.bEnableAction, T0);

        ImGui::BeginDisabled(!Config::AI.ThighCrush.bEnableAction);
        {
            ImGuiEx::SliderF("开始概率", &Config::AI.ThighCrush.fProbability, 1.0f, 100.0f, T1, "%.0f%%");

            ImGui::Spacing();

            ImGuiEx::SliderF("动作间隔", &Config::AI.ThighCrush.fInterval, 1.0f, 10.0f, T2, "每 %.1f 秒");
            ImGuiEx::SliderF("攻击概率", &Config::AI.ThighCrush.fProbabilityHeavy, 0.0f, 100.0f, T3, "%.0f%%");

        }
        ImGui::EndDisabled();

    }

    void DrawAIAction_Hugs() {

        static const std::string HugsInfo = fmt::format(
            fmt::runtime(
				 "拥抱触发需要满足以下条件：\n"
	             "- 体型差必须大于 x{:.2f}\n"
	             "- 体型差必须小于 x2.5\n\n"
	             "只有同时满足这两个条件，AI 才会执行拥抱。\n"
	             "最大体型差可通过拥抱 Perk 提高。"
            ),
            GTS::Action_Hug
        );

        PSString T0 = "启用拥抱动作。";
        PSString T1 = "设置开始拥抱动作的概率。";
        PSString T2 = "允许追随者对其他追随者执行拥抱碾压。";
        PSString T3 = "允许追随者对友好（非战斗）NPC 执行拥抱碾压。";
        PSString T5 = "设置拥抱某人时尝试执行下列拥抱动作的间隔。";
        PSString T6 = "设置执行拥抱治疗的概率。";
        PSString T7 = "设置执行拥抱碾压的概率。";
        PSString T8 = "设置执行拥抱缩小的概率。";
        PSString T9 = "设置对追随者/队友/玩家执行拥抱缩小的概率。";
        PSString T10 = "如果被拥抱角色无法继续缩小，是否将其释放。\n仅适用于追随者/玩家。其他角色总会被释放。";
        PSString T11 = HugsInfo.c_str();

        ImGuiEx::HelpText("为什么没有触发拥抱", T11);
        ImGuiEx::CheckBox("启用动作", &Config::AI.Hugs.bEnableAction, T0);

        ImGui::BeginDisabled(!Config::AI.Hugs.bEnableAction);
        {
            ImGuiEx::SliderF("开始概率", &Config::AI.Hugs.fProbability, 1.0f, 100.0f, T1, "%.0f%%");

            ImGui::Spacing();

            ImGuiEx::CheckBox("允许碾压（追随者和玩家）", &Config::AI.Hugs.bKillFollowersOrPlayer, T2);
            ImGuiEx::CheckBox("允许碾压（友好 NPC）", &Config::AI.Hugs.bKillFriendlies, T3);
            ImGuiEx::CheckBox("过小时停止（追随者和玩家）", &Config::AI.Hugs.bStopIfCantShrink, T10);

            ImGui::Spacing();

            ImGuiEx::SliderF("动作间隔", &Config::AI.Hugs.fInterval, 1.0f, 10.0f, T5, "每 %.1f 秒");
            ImGuiEx::SliderF("治疗概率", &Config::AI.Hugs.fHealProb, 0.0f, 100.0f, T6, "%.0f%%");
            ImGuiEx::SliderF("碾压概率", &Config::AI.Hugs.fKillProb, 0.0f, 100.0f, T7, "%.0f%%");
            ImGuiEx::SliderF("缩小概率", &Config::AI.Hugs.fShrinkProb, 0.0f, 100.0f, T8, "%.0f%%");
            ImGuiEx::SliderF("缩小友方概率", &Config::AI.Hugs.fFriendlyShrinkProb, 0.0f, 100.0f, T9, "%.0f%%");
        }
        ImGui::EndDisabled();

    }

    void DrawAIAction_ButtCrush() {

        PSString T0 = "启用臀部碾压动作。";
        PSString T1 = "设置开始臀部碾压动作的概率。";

        PSString T2 = "如果 AI 决定开始臀部碾压动作，可在快速碾压和锁定目标碾压之间选择。\n"
		                              "在这里调整概率：\n"
		                              "0%% -> 总是执行快速碾压。\n"
		                              "100%% -> 总是执行锁定目标碾压。";

        PSString T3 = "执行锁定目标碾压时，设置尝试执行臀部碾压动作的间隔。";
        PSString T4 = "提高/降低成长概率。通常建议保持较高。";
        PSString T5 = "提高/降低执行臀部碾压的概率。\n"
		                              "注意：执行碾压的概率会根据成长情况在内部提高。\n"
		                              "如果希望追随者经常成长、过一会儿再碾压，请把此值设低。";

        ImGuiEx::CheckBox("启用动作", &Config::AI.ButtCrush.bEnableAction, T0);


        ImGui::BeginDisabled(!Config::AI.ButtCrush.bEnableAction);
        {
            ImGuiEx::SliderF("开始概率", &Config::AI.ButtCrush.fProbability, 1.0f, 100.0f, T1, "%.0f%%");
            ImGuiEx::SliderF("快速/锁定目标碾压概率", &Config::AI.ButtCrush.fButtCrushTypeProb, 0.0f, 100.0f, T2, "%.0f%%");

            ImGui::Spacing();

            ImGuiEx::SliderF("锁定目标动作间隔", &Config::AI.ButtCrush.fInterval, 1.0f, 10.0f, T3, "每 %.1f 秒");
            ImGuiEx::SliderF("成长概率", &Config::AI.ButtCrush.fGrowProb, 0.0f, 100.0f, T4, "%.0f%%");
            ImGuiEx::SliderF("碾压概率", &Config::AI.ButtCrush.fCrushProb, 0.0f, 100.0f, T5, "%.0f%%");
        }
        ImGui::EndDisabled();
    }

    void DrawAIAction_Grab() {

        PSString T0 = "启用抓取动作。";
        PSString T1 = "设置角色在可能时抓取他人的概率。";
        PSString T2 = "角色正抓着某人时，设置尝试执行抓取动作的间隔。";
        PSString T3 = "设置角色决定扔出被抓 NPC 的概率。";
        PSString T4 = "设置角色吞噬被抓 NPC 的概率。";
        PSString T5 = "设置角色碾压被抓 NPC 的概率。";
        PSString T6 = "设置角色把被抓 NPC 放入胸间的概率。";

        PSString T7 = "如果 NPC 位于角色乳沟中，设置开始吸收动作的概率。";
        PSString T8 = "如果 NPC 位于角色乳沟中，设置开始吞噬动作的概率。";
        PSString T9 = "如果 NPC 位于角色乳沟中，设置开始碾压攻击的概率。";
        PSString T10 = "如果 NPC 位于角色乳沟中，设置开始窒息动作的概率。";

        PSString T11 = "设置被抓角色被释放的概率。";
        PSString T12 = "设置停止乳沟动作的概率。";

		PSString T13 = "设置角色开始抓取玩弄动作的概率。";
		PSString T14 = "设置角色在抓取玩弄中执行重碾压的概率。";
		PSString T15 = "设置角色在抓取玩弄中执行吞噬的概率。";
		PSString T16 = "设置角色在抓取玩弄中亲吻的概率。";
		PSString T17 = "设置角色在抓取玩弄亲吻时执行吞噬的概率。";
		PSString T18 = "设置角色在抓取玩弄中戳弄的概率。";
		PSString T19 = "设置角色在抓取玩弄中弹开的概率。";
		PSString T20 = "设置角色在抓取玩弄中夹住对方的概率。";
		PSString T21 = "设置角色在抓取玩弄中开始碾磨的概率。";
		PSString T22 = "设置角色在抓取玩弄中停止碾磨的概率。";
		PSString T23 = "设置角色退出抓取玩弄的概率。";


        ImGuiEx::CheckBox("启用动作", &Config::AI.Grab.bEnableAction, T0);

        ImGui::BeginDisabled(!Config::AI.Grab.bEnableAction);
        {
            ImGuiEx::SliderF("开始概率", &Config::AI.Grab.fProbability, 1.0f, 100.0f, T1, "%.0f%%");
            ImGuiEx::SliderF("动作间隔", &Config::AI.Grab.fInterval, 1.0f, 10.0f, T2, "每 %.1f 秒");

            ImGui::Spacing();
            ImGui::Text("抓取动作");
            ImGuiEx::SliderF("扔出概率", &Config::AI.Grab.fThrowProb, 0.0f, 100.0f, T3, "%.0f%%");
            ImGuiEx::SliderF("吞噬概率", &Config::AI.Grab.fVoreProb, 0.0f, 100.0f, T4, "%.0f%%");
            ImGuiEx::SliderF("碾压概率", &Config::AI.Grab.fCrushProb, 0.0f, 100.0f, T5, "%.0f%%");
            ImGuiEx::SliderF("释放概率", &Config::AI.Grab.fReleaseProb, 0.0f, 100.0f, T11, "%.0f%%");


            ImGui::Spacing();

            ImGui::Text("乳沟动作");
            ImGuiEx::SliderF("放入乳沟概率", &Config::AI.Grab.fCleavageProb, 0.0f, 100.0f, T6, "%.0f%%");
            ImGui::Spacing();
            ImGuiEx::SliderF("乳沟吸收概率", &Config::AI.Grab.fCleavageAbsorbProb, 0.0f, 100.0f, T7, "%.0f%%");
            ImGuiEx::SliderF("乳沟吞噬概率", &Config::AI.Grab.fCleavageVoreProb, 0.0f, 100.0f, T8, "%.0f%%");
            ImGuiEx::SliderF("乳沟碾压概率", &Config::AI.Grab.fCleavageAttackProb, 0.0f, 100.0f, T9, "%.0f%%");
            ImGuiEx::SliderF("乳沟窒息概率", &Config::AI.Grab.fCleavageSuffocateProb, 0.0f, 100.0f, T10, "%.0f%%");
            ImGuiEx::SliderF("乳沟停止概率", &Config::AI.Grab.fCleavageStopProb, 0.0f, 100.0f, T12, "%.0f%%");

            ImGui::Spacing();

            ImGui::Text("抓取玩弄动作");
            ImGuiEx::SliderF("GP 开始概率", &Config::AI.Grab.fGrabPlayStartProb, 0.0f, 100.0f, T13, "%.0f%%");
            ImGui::Spacing();
            ImGuiEx::SliderF("GP 重碾压概率", &Config::AI.Grab.fGrabPlayHeavyCrushProb, 0.0f, 100.0f, T14, "%.0f%%");
            ImGuiEx::SliderF("GP 吞噬概率", &Config::AI.Grab.fGrabPlayVoreProb, 0.0f, 100.0f, T15, "%.0f%%");
            ImGuiEx::SliderF("GP 亲吻概率", &Config::AI.Grab.fGrabPlayKissProb, 0.0f, 100.0f, T16, "%.0f%%");
            ImGuiEx::SliderF("GP 亲吻吞噬概率", &Config::AI.Grab.fGrabPlayKissVoreProb, 0.0f, 100.0f, T17, "%.0f%%");
            ImGuiEx::SliderF("GP 戳弄概率", &Config::AI.Grab.fGrabPlayPokeProb, 0.0f, 100.0f, T18, "%.0f%%");
            ImGuiEx::SliderF("GP 弹开概率", &Config::AI.Grab.fGrabPlayFlickProb, 0.0f, 100.0f, T19, "%.0f%%");
            ImGuiEx::SliderF("GP 夹击概率", &Config::AI.Grab.fGrabPlaySandwichProb, 0.0f, 100.0f, T20, "%.0f%%");

            ImGuiEx::SliderF("GP 开始碾磨概率", &Config::AI.Grab.fGrabPlayGrindStartProb, 0.0f, 100.0f, T21, "%.0f%%");
            ImGuiEx::SliderF("GP 停止碾磨概率", &Config::AI.Grab.fGrabPlayGrindStopProb, 0.0f, 100.0f, T22, "%.0f%%");

            ImGuiEx::SliderF("GP 退出概率", &Config::AI.Grab.fGrabPlayExitProb, 0.0f, 100.0f, T23, "%.0f%%");

        }
        ImGui::EndDisabled();
    }

    void DrawAIAction_Calamity() {
        PSString TCHelp = "Tiny Calamity（微小灾厄）通常是玩家专属的喊声效果。\n"
                          "- 模组默认没有给 NPC 施加该效果的途径";
        PSString WCHelp = "狂怒灾厄是通常仅玩家可用的一击必杀动作。\n"
                          "- 只能对生命值较低的人形敌人使用\n"
                          "- 且需要 Tiny Calamity 处于激活状态";
        PSString T0 = "允许 AI 开始狂怒灾厄动作。";
        PSString T1 = "设置开始狂怒灾厄动作的概率。";
        PSString TC = "当 Tiny Calamity 激活时：\n"
                        "- 开启：NPC 先缩小敌人，再做体型动作\n"
                        "- 关闭：NPC 跳过缩小，直接播放动作\n"
                        "建议保持开启，同体型动作会非常违和。";
        
        ImGui::Text("Tiny Calamity");
        ImGuiEx::HelpText("什么是 Tiny Calamity", TCHelp);
        ImGuiEx::CheckBox("Tiny Calamity：启用先缩小动画", &Config::AI.bCalamityShrinksFirst, TC);
        ImGui::Spacing();
        ImGui::Text("狂怒灾厄");
        ImGuiEx::HelpText("什么是狂怒灾厄", WCHelp);
        ImGuiEx::CheckBox("启用动作", &Config::AI.WrathfulCalamity.bEnableAction, T0);
        ImGuiEx::SliderF("开始概率", &Config::AI.WrathfulCalamity.fProbability, 1.0f, 100.0f, T1, "%.0f%%", !Config::AI.WrathfulCalamity.bEnableAction);
    }

} 

namespace GTS {

	CategoryAI::CategoryAI() {
		m_name = "AI";
	}

    void CategoryAI::DrawLeft() {

        ImUtil_Unique
		{

            PSString T0 = "全局启用/禁用追随者动作 AI。";

            PSString T1 = "设置追随者尝试开始新 GT 动作的时间间隔。\n"
                          "这不保证追随者每隔 x 秒一定会执行动作。\n"
                          "它只改变尝试开始某个动作的间隔。\n"
                          "是否实际执行取决于右侧各动作配置的概率。";

            PSString T2 = "允许 AI 以玩家为目标。";
            PSString T3 = "允许 AI 以其他追随者为目标。";
            PSString T4 = "仅当 NPC 与执行者敌对时，才允许 AI 执行动作。";
            PSString T5 = "仅在追随者处于战斗中时启用动作 AI。";

            PSString T6 = "当追随者体型较大时，按体型阻止其使用普通攻击。\n"
                          "- 不攻击的概率会随体型增大而提高。\n"
                          "- 仅在敌人接近追随者时生效。";

            PSString T7 = "启用后，即使附近没有敌人，也总是阻止追随者普通攻击。";

            PSString THelp = "GT AI 会按可配置的时间间隔检查可开始的 GT 动作。\n"
                             "AI 是否执行动作取决于对应动作的概率设置。\n"
                             "如果概率较低，该动作就不太容易开始。\n"
                             "当所有动作概率都较低时，AI 更可能什么都不做。\n"
                             "部分动作（如拥抱或抓取）拥有自己的子动作，这些子动作也按同样逻辑工作，\n"
                             "可在本设置页右侧面板调整。";

            if (ImGui::CollapsingHeader("AI 设置",ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::HelpText("这是如何工作的", THelp);
                ImGuiEx::CheckBox("启用 AI",&Config::AI.bEnableActionAI, T0);

                {
                    ImGui::BeginDisabled(!Config::AI.bEnableActionAI);

                    ImGuiEx::SliderF("尝试开始新动作",&Config::AI.fMasterTimer, 2.0f, 15.0f, T1 ,"每 %.1f 秒");
                    ImGui::Spacing();

                    ImGuiEx::CheckBox("目标包含玩家", &Config::AI.bAllowPlayer, T2);
                    ImGui::SameLine();
                    ImGuiEx::CheckBox("目标包含其他追随者", &Config::AI.bAllowFollowers, T3);
                    ImGui::SameLine();
                    ImGuiEx::CheckBox("仅敌对目标", &Config::AI.bHostileOnly, T4);

                    ImGuiEx::CheckBox("仅战斗中启用 AI",&Config::AI.bCombatOnly, T5);
                    const bool FollowersGTOnly = Config::AI.bFollowersGTOnly;
                    ImGuiEx::CheckBox("体型较大时禁用普通攻击", &Config::AI.bDisableAttacks, T6, FollowersGTOnly);
                    ImGuiEx::CheckBox("体型较大时总是禁用普通攻击", &Config::AI.bAlwaysDisableAttacks, T7, FollowersGTOnly || !Config::AI.bDisableAttacks);

                    ImGui::EndDisabled();
                }

                ImGui::Spacing();
            }
        }

        ImUtil_Unique
		{

            PSString T0 = "切换追随者超过 x1.25 体型后，是否进一步降低移动速度。\n"
                            "- 此开关会根据体型降低追随者冲刺概率，使其更倾向于行走而不是冲刺。\n"
                            "- 会修改 SpeedMult Actor Value，最多可降低 15。\n\n"
                            "- 此开关需要动态动画速度开关启用后才会生效。";
            PSString T2 = "切换追随者超过 x1.25 体型后，是否降低转身速度。\n\n"
                            "- 此开关需要动态动画速度开关启用后才会生效。";
            PSString T3 = "切换追随者是否记录腿/手的移动速度。\n"
                             "- 启用后，追随者只是站着不动时不会推开他人。\n"
                             "- 由于会每帧为每个追随者记录数据，可能对 FPS 有负面影响。";
            PSString T4 = "切换其他 NPC 接近 GT 角色时是否会进入逃离状态。";

            if (ImGui::CollapsingHeader("杂项设置",ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("降低移动速度 AV", &Config::AI.bSlowMovementDown, T0);
                ImGuiEx::CheckBox("降低战斗转身速度", &Config::AI.bSlowRotationDown, T2);
                ImGuiEx::CheckBox("记录节点移动速度数据", &Config::AI.bRecordBoneSpeedData, T3);
                ImGuiEx::CheckBox("角色恐慌", &Config::AI.bPanic, T4);
                ImGui::Spacing();
            }
		}

    }

    void CategoryAI::DrawRight() {

		static ImGuiEx::CollapsingTabHeader ActionHeader (
            "AI 动作设置",
			{
				"吞噬",
				"踩踏",
                "踢击/挥击",
                "大腿夹击",
                "大腿压碎",
                "拥抱",
                "臀部压碎",
                "抓取",
                "Tiny Calamity",
			}
        );

        ActionHeader.SetDisabledState(!Config::AI.bEnableActionAI);

        if (ImGuiEx::BeginCollapsingTabHeader(ActionHeader)) {
            // Content based on active tab
            switch (ActionHeader.GetActiveTab()) {
                case 0: DrawAIAction_Vore();          break;
                case 1: DrawAIAction_Stomps();        break;
                case 2: DrawAIAction_KickSwipe();     break;
                case 3: DrawAIAction_ThighSandwich(); break;
                case 4: DrawAIAction_ThighCrush();    break;
                case 5: DrawAIAction_Hugs();          break;
                case 6: DrawAIAction_ButtCrush();     break;
                case 7: DrawAIAction_Grab();          break;
                case 8: DrawAIAction_Calamity();      break;
				default:                              break;
            }
        }
        ImGuiEx::EndCollapsingTabHeader(ActionHeader);
    }
}
