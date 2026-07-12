#include "UI/Windows/Settings/Categories/Balance.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/ConditionalHeader.hpp"

#include "Config/Config.hpp"

#include "UI/Controls/Text.hpp"


namespace GTS {

	CategoryBalance::CategoryBalance() {
		m_name = "平衡";
	}

    void CategoryBalance::DrawLeft() {

        ImUtil_Unique
		{

            PSString T0 = "启用/禁用平衡模式。";
            PSString T1 = "启用平衡模式后，对所有体型增长施加的惩罚倍率。";
            PSString T2 = "持续缩小倍率，战斗内外都会生效。";
            PSString T3 = "战斗中会额外乘上这个缩小倍率。";
            PSString T4 = "影响你在受击时损失的体型量。";

            PSString THelp = "平衡模式会显著提高成长难度。\n"
                             "- 敌人在仍有体力时，会更能抵抗尺寸伤害。\n"
                             "- 你在受击时以及脱离战斗后会持续缩小。\n\n"
                             "- 此外，所有属性成长会额外削弱 50%，并且部分设置会被禁用。\n"
                             "- 启用平衡模式时，无法跳过任务线。";

            if (ImGui::CollapsingHeader("平衡模式", ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::HelpText("什么是平衡模式", THelp);
                ImGuiEx::CheckBox("启用平衡模式", &Config::Balance.bBalanceMode, T0);

                ImGui::BeginDisabled(!Config::Balance.bBalanceMode);

                ImGuiEx::SliderF("成长惩罚", &Config::Balance.fBMSizeGainPenaltyMult, 1.0f, 10.0f, T1, "%.2fx");
                ImGuiEx::SliderF("持续缩小倍率", &Config::Balance.fBMShrinkRate, 0.01f, 10.0f, T2, "%.2fx");
                ImGuiEx::SliderF("战斗中缩小倍率", &Config::Balance.fBMShrinkRateCombat, 0.01f, 1.0f, T3, "%.2fx");
                ImGuiEx::SliderF("受击缩小倍率", &Config::Balance.fBMShrinkOnHitMult, 0.01f, 2.0f, T4, "%.2fx");

                ImGui::EndDisabled();

                ImGui::Spacing();
            }
        }


        ImUtil_Unique
		{

            PSString T0 = "Controls how much character size affects movement speed.\n"
            "\n"
            "- Lower values reduce movement speed, but may cause foot sliding (ice skating).\n"
            "- Higher values better match model movement, but make large characters move faster.\n"
            "\n"
            "Default: 1.0 (Recommended)";

            if (ImGui::CollapsingHeader("Movement Speed", ImUtil::HeaderFlagsDefaultOpen)) {

                {
                    ImGui::Text("Movement Speed Scaling");
                    ImGuiEx::SliderF("Size Influence", &Config::Balance.fSizeSpeedPercentage, 0.3f, 1.0f, T0, "%.2fx");
                }

                ImGui::Spacing();
            }
        }
        ImUtil_Unique
		{

            PSString T0 = "Controls how much character size affects animation speed.\n"
            "Default: 1.0 (Recommended)";

            if (ImGui::CollapsingHeader("Animation Speed", ImUtil::HeaderFlagsDefaultOpen)) {

                {
                    ImGui::Text("Animation Speed Scaling");
                    ImGuiEx::SliderF("Size Influence", &Config::Balance.fAnimSpeedInfluence, 0.1f, 1.0f, T0, "%.2fx");
                }

                ImGui::Spacing();
            }
        }

    	// ---- Misc

        ImUtil_Unique
		{

            PSString T0 = "决定玩家是否会受到友方的尺寸相关伤害。";
            PSString T1 = "决定追随者是否会受到友方的尺寸相关伤害。";
            PSString T2 = "决定玩家和追随者是否会因为他人的体型而被击退/布娃娃化。";
            PSString T3 = "决定其他 NPC 是否会因为他人的体型而被击退/布娃娃化。";

            if (ImGui::CollapsingHeader("杂项设置", ImUtil::HeaderFlagsDefaultOpen)) {

                {
                    ImGui::Text("友方尺寸伤害免疫");
                    ImGuiEx::CheckBox("玩家", &Config::Balance.bPlayerFriendlyImmunity, T0);
                    ImGui::SameLine();
                    ImGuiEx::CheckBox("追随者", &Config::Balance.bFollowerFriendlyImmunity, T1);
                }

                ImGui::Spacing();

                {

                    ImGui::Text("尺寸击退/失衡");
                    ImGuiEx::CheckBox("允许作用于 NPC", &Config::Balance.bAllowOthersStagger, T3);
                    ImGui::SameLine();
                    ImGuiEx::CheckBox("允许作用于友军", &Config::Balance.bAllowFriendlyStagger, T2);

                }

                ImGui::Spacing();
            }
        }

		ImUtil_Unique
        {

            PSString T1 = "把部分玩家已获得的 Perk 分享给追随者（如果他们还没有）。\n"
                          "（影响最大体型的 Perk 不会共享）";

			if (ImGui::CollapsingHeader("Perk 设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("向追随者共享 Perk", &Config::Balance.bSharePerks, T1);
                ImGui::Spacing();
            }
        }

    }

    void CategoryBalance::DrawRight() {

        ImUtil_Unique
		{

            PSString T0 = "修改所有体型成长使用的计算公式。";
            PSString T1 = "调整全局的体型增减倍率。";

            PSString THelp = "体型成长模式决定你如何获得最大体型。\n"
                             "普通模式下，你的最大体型由以下因素决定：\n"
                             "当前 Perk、GTS 技能等级、角色等级、吸收的精华（药水或击杀龙获得）以及任务进度。\n"
                             "\"质量模式\" 下，你的最大体型更多取决于吸收/吞食/击杀的累积量。\n"
                             "质量模式会从 1.0x 开始，随着你吸收的目标增多，逐步成长到常规技能上限，\n"
                             "如果启用了扩展页中的体型上限规则，则以命中的手动规则为准。";

            if (ImGui::CollapsingHeader("体型选项", ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::HelpText("这是什么", THelp);

                if (ImGuiEx::ComboEx<LSizeMode_t>("成长模式", Config::Balance.sSizeMode, T0)) {
                    if (Config::Balance.sSizeMode == "kNormal") Config::Balance.fSpellEfficiency = 0.55f;
                    if (Config::Balance.sSizeMode == "kMassBased") Config::Balance.fSpellEfficiency = 0.33f;
                }

                ImGuiEx::SliderF("效率倍率", &Config::Balance.fSpellEfficiency, 0.1f, 1.0f, T1, "%.2fx");

                ImGui::Spacing();
            }
        }

        // ---- Multipiers

        ImUtil_Unique
		{

            PSString T0 = "修改尺寸相关动作造成的伤害倍率。";
            PSString T1 = "修改普通近战和法术因体型获得的伤害增幅。";
            PSString T2 = "调整尺寸经验的获取速度。";
            PSString T3 = "修改体型带来的负重加成倍率。";

            if (ImGuiEx::ConditionalHeader("倍率", "平衡模式已启用", true)) {

                ImGuiEx::SliderF("尺寸伤害倍率", &Config::Balance.fSizeDamageMult, 0.02f, 2.0f, T0, "%.2fx");
                ImGuiEx::SliderF("武器/法术伤害倍率", &Config::Balance.fStatBonusDamageMult, 0.02f, 2.0f, T1, "%.2fx");
                ImGuiEx::SliderF("负重倍率", &Config::Balance.fStatBonusCarryWeightMult, 0.02f, 2.0f, T3, "%.2fx");
                ImGuiEx::SliderF("经验倍率", &Config::Balance.fExpMult, 0.02f, 5.0f, T2, "%.2fx");
            }
        }
    }
}
