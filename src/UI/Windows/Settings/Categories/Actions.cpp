#include "UI/Windows/Settings/Categories/Actions.hpp"

#include "Config/Config.hpp"
#include "Config/ConfigModHandler.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Core/ImUtil.hpp"

namespace GTS {

	CategoryActions::CategoryActions() {
		m_name = "动作";
	}

	void CategoryActions::DrawLeft() {

        // ------- Misc Settings
        
		ImUtil_Unique
		{

			PSString T0 = "允许在部分动作中调整视野（例如 Second Wind）。";
			PSString T1 = "在部分动画动作期间追踪双足骨架节点位置。";

			if (ImGui::CollapsingHeader("杂项", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::CheckBox("启用 FOV 调整", &Config::General.bEnableFOVEdits, T0);
				if (ImGuiEx::CheckBox("动作期间追踪骨骼", &Config::General.bTrackBonesDuringAnim, T1)) {
                    ConfigModHandler::DoCameraStateReset();
				}

                ImGui::Spacing();
			}
		}

		//------ Visuals

        ImUtil_Unique
		{

            PSString T0 = "在部分动作中显示爱心粒子效果。";
            PSString T1 = "在 NPC 头顶显示/隐藏可执行 GTS 动作的提示图标。";

            if (ImGui::CollapsingHeader("视觉效果", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("爱心效果",&Config::General.bShowHearts, T0);
                ImGui::SameLine();
                ImGuiEx::CheckBox("显示动作图标",&Config::General.bShowIcons, T1);
                ImGui::Spacing();
            }
        }

		//----- Sneak Animations

        ImUtil_Unique
		{

            PSString T1 = "仅对玩家用爬行替换潜行。\n（存档独立设置）\n"
						  "注意：如果潜行/爬行过渡动画关闭，将不会自动在爬行与潜行状态之间切换。";

	PSString T2 = "对追随者用爬行替换潜行。\n（存档独立设置）\n"
						  "注意：如果潜行/爬行过渡动画关闭，将不会自动在爬行与潜行状态之间切换。";

            PSString T3 = "本模组在进入/退出潜行或爬行状态时添加了细微的过渡动画。\n"
						  "此开关用于启用或禁用这些过渡动画。\n";


            if (ImGui::CollapsingHeader("潜行/爬行", ImUtil::HeaderFlagsDefaultOpen)) {

                bool PlayerBusy = AnimationVars::General::IsTransitioning(PlayerCharacter::GetSingleton());
                bool FollowersBusy = false;

                for (const auto& Fol : FindTeammates()) {
                    if (Fol) {
                        if (AnimationVars::General::IsTransitioning(Fol)) {
                            FollowersBusy = true;
                            break;
                        }
                    }
                }

                ImGui::Text("用爬行替换潜行");
                ImGuiEx::CheckBox("玩家##CrawlToggle", &Persistent::EnableCrawlPlayer.value, T1, PlayerBusy);
                ImGui::SameLine();
                ImGuiEx::CheckBox("追随者##CrawlToggle", &Persistent::EnableCrawlFollower.value, T2, FollowersBusy);

                ImGui::Spacing();

                ImGui::Text("潜行过渡动画");
                ImGuiEx::CheckBox("玩家##STransToggle", &Config::Gameplay.ActionSettings.bSneakTransitions, T3);
                ImGui::SameLine();
                ImGuiEx::CheckBox("追随者##STransToggle", &Config::Gameplay.ActionSettings.bSneakTransitionsOther, T3);

                ImGui::Spacing();
            }
        }

        //----- Stomps/Kicks

        ImUtil_Unique
		{

	PSString T0 = "提高/降低执行脚下踩踏时触发脚底碾磨动画的概率。";

	PSString T1 = "启用后：\n"
				  "将 SonderBain 制作的轻踩（非脚下踩踏）动画替换为 NickNack 制作的不同版本。";

            PSString T3 = "切换追随者执行踢击等动作时，是否会让玩家进入布娃娃状态。";
           

			if (ImGui::CollapsingHeader("踩踏/踢击", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::SliderF("脚下踩踏触发碾磨概率", &Config::Gameplay.ActionSettings.fPlayerUnderstompGrindChance, 0.0f, 100.0f, T0, "%.0f%%");
                ImGuiEx::CheckBox("玩家替代踩踏动画", &Config::Gameplay.ActionSettings.bStompAlternative, T1);
			    ImGui::SameLine();
                ImGuiEx::CheckBox("NPC 替代踩踏动画", &Config::Gameplay.ActionSettings.bStomAlternativeOther, T1);
                ImGuiEx::CheckBox("追随者踢击影响玩家", &Config::Gameplay.ActionSettings.bEnablePlayerPushBack, T3);
			    ImGui::Spacing();
			}
        }
	}

	void CategoryActions::DrawRight() {

        //----- Vore Settings

        ImUtil_Unique
		{

			PSString T1 = "调整吞噬后获得的成长量。";
            PSString T2 = "执行任何吞噬动作时启用 Skyrim 自由镜头。";
            PSString T3 = "吞噬后提高原版角色体重。";
            PSString T4 = "允许吞噬昆虫。";
            PSString T5 = "允许吞噬亡灵角色（例如尸鬼）。";

            PSString T6 = "启用 Devourment 兼容时，切换 GTS 是否对玩家和队友执行 DV 的 Endo，\n"
			              "而不是致命吞噬。";


            PSString T7 = "设置吞噬吸收的基础持续时间。实际消化时间可能因 Perk 而缩短。";
						  

            if (ImGui::CollapsingHeader("吞噬设置", ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::SliderF("吞噬成长倍率", &Config::Gameplay.ActionSettings.fVoreGainMult, 0.1f, 3.0f, T1, "%.1fx");
                ImGuiEx::SliderU16("基础消化时间", &Config::Gameplay.ActionSettings.iVoreDigestSpeedTime, 40, 360, T7, "%d 秒");
                ImGuiEx::CheckBox("允许昆虫", &Config::Gameplay.ActionSettings.bAllowInsects, T4);
                ImGui::SameLine();
                ImGuiEx::CheckBox("允许亡灵", &Config::Gameplay.ActionSettings.bAllowUndead, T5);
                ImGuiEx::CheckBox("吞噬期间启用自由镜头", &Config::Gameplay.ActionSettings.bVoreFreecam, T2);
                ImGuiEx::CheckBox("吞噬后增加角色体重", &Config::Gameplay.ActionSettings.bVoreWeightGain, T3);
                ImGuiEx::CheckBox("对追随者/玩家使用 Endo 吞噬", &Config::Gameplay.ActionSettings.bDVDoEndoOnTeam, T6);

                ImGui::Spacing();
            }
        }

		//----- Grab Settings

        ImUtil_Unique
		{

            PSString T1 = "切换初始抓取是否视为敌对行为：\n"
                          "- true = 抓取时会进入战斗\n"
                          "- false = NPC 不会进入战斗";
            

            PSString T2 = "调整亲吻吞噬期间角色的位置。\n"
                          "偏移会受到体型差影响。";

            if (ImGui::CollapsingHeader("抓取设置", ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::CheckBox("初始抓取视为敌对", &Config::Gameplay.ActionSettings.bGrabStartIsHostile, T1);

                ImGui::Spacing();

                ImGui::Text("抓取玩弄：亲吻吞噬偏移");
                ImGuiEx::SliderF("上/下", &Config::Gameplay.ActionSettings.fGrabPlayVoreOffset_Z, -15.0f, 15.0f, T2, "%.2f");

                ImGui::Spacing();
            }

        }

        //----- Hug Settings

        ImUtil_Unique
		{

            PSString T0 = "切换非致命拥抱动作（如拥抱治疗或拥抱缩小）是否会进入战斗。";

            PSString T1 = "切换拥抱治疗到满血后是否释放被抱住的角色。\n"
			              "仅适用于玩家/追随者。";

            if (ImGui::CollapsingHeader("拥抱设置", ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::CheckBox("非致命拥抱视为敌对", &Config::Gameplay.ActionSettings.bNonLethalHugsHostile, T0);
                ImGui::SameLine();
                ImGuiEx::CheckBox("满血后停止拥抱治疗", &Config::Gameplay.ActionSettings.bHugsStopAtFullHP, T1);

	ImGui::Spacing();
            }
        }

        ImUtil_Unique
		{

            if (ImGui::CollapsingHeader("乳沟设置", ImUtil::HeaderFlagsDefaultOpen)) {

                PSString T1 = "调整乳沟动作期间角色的位置。\n"
                              "上/下 | 前/后";

                ImGuiEx::SliderF2("位置偏移", &Config::Gameplay.ActionSettings.f2CleavageOffset.at(0), -15.0f, 15.0f, T1, "%.2f");
                ImGui::Spacing();
            }
        }
	}
}
