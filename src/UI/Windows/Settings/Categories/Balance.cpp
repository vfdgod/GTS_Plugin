#include "UI/Windows/Settings/Categories/Balance.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/ConditionalHeader.hpp"

#include "Managers/MaxSizeManager.hpp"
#include "Config/Config.hpp"

#include "UI/Controls/Text.hpp"

#include <algorithm>


namespace GTS {

	namespace {
		constexpr float SizeRuleFixedLimitMin = 0.05f;
		constexpr float SizeRuleFixedLimitMax = 255.0f;
		template <class Enum>
		std::string EnumName(Enum a_value) {
			return std::string(magic_enum::enum_name(a_value));
		}

		bool TryParseRuleTarget(const std::string& a_value, LSizeLimitRuleTarget_t& a_out) {
			if (auto parsed = magic_enum::enum_cast<LSizeLimitRuleTarget_t>(a_value); parsed.has_value()) {
				a_out = *parsed;
				return true;
			}
			return false;
		}

		bool TryParseRuleMode(const std::string& a_value, LSizeLimitRuleMode_t& a_out) {
			if (auto parsed = magic_enum::enum_cast<LSizeLimitRuleMode_t>(a_value); parsed.has_value()) {
				a_out = *parsed;
				return true;
			}
			return false;
		}

		LSizeLimitRuleTarget_t GetRuleTarget(const SizeLimitRule_t& a_rule) {
			LSizeLimitRuleTarget_t target = LSizeLimitRuleTarget_t::kHumanoidNPC;
			TryParseRuleTarget(a_rule.sTarget, target);
			return target;
		}

		LSizeLimitRuleMode_t GetRuleMode(const SizeLimitRule_t& a_rule) {
			LSizeLimitRuleMode_t mode = LSizeLimitRuleMode_t::kFixedLimit;
			TryParseRuleMode(a_rule.sMode, mode);
			return mode;
		}

		const char* GetRuleTargetLabel(LSizeLimitRuleTarget_t a_target) {
			switch (a_target) {
				case LSizeLimitRuleTarget_t::kPlayer:
					return "玩家";
				case LSizeLimitRuleTarget_t::kFollower:
					return "追随者";
				case LSizeLimitRuleTarget_t::kHostile:
					return "敌对目标";
				case LSizeLimitRuleTarget_t::kImportant:
					return "重要角色";
				case LSizeLimitRuleTarget_t::kHumanoidNPC:
					return "普通人形 NPC";
				case LSizeLimitRuleTarget_t::kAnimal:
					return "动物";
				case LSizeLimitRuleTarget_t::kCreature:
					return "怪物 / Creature";
				case LSizeLimitRuleTarget_t::kDragon:
					return "龙";
				case LSizeLimitRuleTarget_t::kGiantMammoth:
					return "巨人 / 猛犸";
				case LSizeLimitRuleTarget_t::kMechanical:
					return "机械 / 矮人造物";
				default:
					return "未知分类";
			}
		}

		const char* GetRuleTargetTooltip(LSizeLimitRuleTarget_t a_target) {
			switch (a_target) {
				case LSizeLimitRuleTarget_t::kPlayer:
					return "只匹配玩家自己。";
				case LSizeLimitRuleTarget_t::kFollower:
					return "匹配追随者/队友类目标。";
				case LSizeLimitRuleTarget_t::kHostile:
					return "匹配当前与玩家敌对，或战斗目标互相指向彼此的非玩家目标。";
				case LSizeLimitRuleTarget_t::kImportant:
					return "匹配带 Essential/重要角色标记的非玩家目标。";
				case LSizeLimitRuleTarget_t::kHumanoidNPC:
					return "匹配普通人形 NPC。若它同时也是追随者、敌对或重要角色，顺序由规则列表决定。";
				case LSizeLimitRuleTarget_t::kAnimal:
					return "匹配带 AnimalKeyword 的目标。猛犸也可能命中这类，最终以规则顺序为准。";
				case LSizeLimitRuleTarget_t::kCreature:
					return "匹配带 CreatureKeyword 的怪物目标。龙等特殊种类若同时命中，也由顺序决定。";
				case LSizeLimitRuleTarget_t::kDragon:
					return "单独匹配龙类目标。";
				case LSizeLimitRuleTarget_t::kGiantMammoth:
					return "单独匹配巨人和猛犸。";
				case LSizeLimitRuleTarget_t::kMechanical:
					return "匹配 Dwemer / 机械造物。";
				default:
					return "";
			}
		}

		const char* GetRuleModeLabel(LSizeLimitRuleMode_t a_mode) {
			switch (a_mode) {
				case LSizeLimitRuleMode_t::kNaturalCeiling:
					return "自然体型封顶";
				case LSizeLimitRuleMode_t::kNaturalLock:
					return "锁定自然体型";
				case LSizeLimitRuleMode_t::kFixedLimit:
					return "固定上限";
				case LSizeLimitRuleMode_t::kActionFit:
					return "动作适应";
				case LSizeLimitRuleMode_t::kSystemAuto:
					return "原系统自动";
				case LSizeLimitRuleMode_t::kUnlimited:
					return "无限";
				default:
					return "未知模式";
			}
		}

		const char* GetRuleModeTooltip(LSizeLimitRuleMode_t a_mode) {
			switch (a_mode) {
				case LSizeLimitRuleMode_t::kNaturalCeiling:
					return "只阻止目标高于自然体型，允许被继续缩小。";
				case LSizeLimitRuleMode_t::kNaturalLock:
					return "始终维持在自然体型，放大和缩小都会被程序逐步拉回。";
				case LSizeLimitRuleMode_t::kFixedLimit:
					return "将最大体型限制在你设置的数值；若目标当前高于该值，会被逐步压回。";
				case LSizeLimitRuleMode_t::kActionFit:
					return "动态跟随当前玩家体型，为常规动作保留可用上限。玩家不提供这个模式。";
				case LSizeLimitRuleMode_t::kSystemAuto:
					return "命中这条规则后停止继续匹配，不写额外 override，改为沿用该目标原本的 GTS 成长公式。";
				case LSizeLimitRuleMode_t::kUnlimited:
					return "命中这条规则后直接取消额外上限。";
				default:
					return "";
			}
		}

		const char* GetRecallFilterModeLabel(LShrinkRecallFilterMode_t a_mode) {
			switch (a_mode) {
				case LShrinkRecallFilterMode_t::kAllShrunken:
					return "全部缩小角色";
				case LShrinkRecallFilterMode_t::kCustomTargets:
					return "自定义分类";
				default:
					return "未知模式";
			}
		}

		const char* GetRecallFilterModeTooltip(LShrinkRecallFilterMode_t a_mode) {
			switch (a_mode) {
				case LShrinkRecallFilterMode_t::kAllShrunken:
					return "只要是非玩家、当前已缩小、活着且已加载的角色，都允许被移动。";
				case LShrinkRecallFilterMode_t::kCustomTargets:
					return "只有命中你在下方勾选分类、并且当前已缩小的角色，才会被移动。";
				default:
					return "";
			}
		}

		const char* GetRecallPlacementLabel(LShrinkRecallPlacement_t a_mode) {
			switch (a_mode) {
				case LShrinkRecallPlacement_t::kRing:
					return "环形";
				case LShrinkRecallPlacement_t::kFront:
					return "面前";
				default:
					return "未知模式";
			}
		}

		const char* GetRecallPlacementTooltip(LShrinkRecallPlacement_t a_mode) {
			switch (a_mode) {
				case LShrinkRecallPlacement_t::kRing:
					return "把目标分散摆在玩家周围的同心圆上，最稳妥，不容易全部堆到正前方。";
				case LShrinkRecallPlacement_t::kFront:
					return "把目标分散摆在玩家前方的数排位置，更方便你正面观察和处理。";
				default:
					return "";
			}
		}

		bool TryParseRecallFilterMode(const std::string& a_value, LShrinkRecallFilterMode_t& a_out) {
			if (auto parsed = magic_enum::enum_cast<LShrinkRecallFilterMode_t>(a_value); parsed.has_value()) {
				a_out = *parsed;
				return true;
			}
			return false;
		}

		bool TryParseRecallPlacement(const std::string& a_value, LShrinkRecallPlacement_t& a_out) {
			if (auto parsed = magic_enum::enum_cast<LShrinkRecallPlacement_t>(a_value); parsed.has_value()) {
				a_out = *parsed;
				return true;
			}
			return false;
		}

		bool IsRecallTargetSelected(const std::vector<std::string>& a_targets, LSizeLimitRuleTarget_t a_target) {
			const std::string_view targetName = magic_enum::enum_name(a_target);
			return std::find(a_targets.begin(), a_targets.end(), targetName) != a_targets.end();
		}

		void SetRecallTargetSelected(std::vector<std::string>& a_targets, LSizeLimitRuleTarget_t a_target, bool a_selected) {
			const std::string targetName = EnumName(a_target);
			const auto iter = std::find(a_targets.begin(), a_targets.end(), targetName);

			if (a_selected) {
				if (iter == a_targets.end()) {
					a_targets.push_back(targetName);
				}
			}
			else if (iter != a_targets.end()) {
				a_targets.erase(iter);
			}
		}

		bool RuleTargetSupportsActionFit(LSizeLimitRuleTarget_t a_target) {
			return a_target != LSizeLimitRuleTarget_t::kPlayer;
		}

		SizeLimitRule_t MakeDefaultRule(LSizeLimitRuleTarget_t a_target) {
			SizeLimitRule_t rule;
			rule.bEnabled = true;
			rule.sTarget = EnumName(a_target);
			rule.sMode = EnumName(LSizeLimitRuleMode_t::kFixedLimit);
			rule.fValue = 1.0f;
			return rule;
		}

		bool TargetAlreadyUsed(const std::vector<SizeLimitRule_t>& a_rules, LSizeLimitRuleTarget_t a_target) {
			for (const auto& rule : a_rules) {
				if (GetRuleTarget(rule) == a_target) {
					return true;
				}
			}
			return false;
		}

		std::vector<LSizeLimitRuleTarget_t> CollectAddableTargets(const std::vector<SizeLimitRule_t>& a_rules) {
			std::vector<LSizeLimitRuleTarget_t> targets;
			targets.reserve(10);

			for (int i = 0; i < static_cast<int>(LSizeLimitRuleTarget_t::kTotal); ++i) {
				const auto target = static_cast<LSizeLimitRuleTarget_t>(i);
				if (!TargetAlreadyUsed(a_rules, target)) {
					targets.push_back(target);
				}
			}

			return targets;
		}
	}

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
                             "或者在你解锁对应 Perk 后，成长到你手动设定的最大上限。";

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

        ImUtil_Unique 
		{

            PSString THelp = "最大体型并不只由这些滑条决定。\n"
                             "它还会受到以下因素影响：\n"
                             "- 游戏缩放（SetScale）\n"
                             "- 角色自然体型";

        	const bool HasPerk = Runtime::HasPerk(PlayerCharacter::GetSingleton(), Runtime::PERK.GTSPerkColossalGrowth);
            const bool Unlock = Persistent::UnlockMaxSizeSliders.value;

            std::string DisableReason = "";
            if (Config::Balance.bBalanceMode) {
                DisableReason += "平衡模式已启用";
            }
            else if (!HasPerk) {
                DisableReason += "缺少 Perk：\"巨型成长\"";
            }
            else if (!Unlock) {
                DisableReason += "需要先执行控制台命令：gts unlimited";
            }

            bool showMovedSizeLimitPage = false;
            if (showMovedSizeLimitPage && ImGuiEx::ConditionalHeader("体型上限", DisableReason, HasPerk && Unlock && !Config::Balance.bBalanceMode)) {
                EnsureSizeLimitRulesInitialized();
                ImGuiEx::HelpText("最大体型由什么决定", THelp);
                ImGuiEx::HelpText(
                    "规则如何工作",
                    "从上到下检查规则，第一条命中后立即停止，不再看后面的分类。\n"
                    "未命中任何规则时：不处理，不写额外 override，保留原系统体型逻辑。\n"
                    "玩家不提供“动作适应”模式。"
                );

                auto& rules = Config::Balance.SizeLimitRules;
                auto addableTargets = CollectAddableTargets(rules);
                static int addTargetIndex = 0;

                if (!addableTargets.empty()) {
                    if (addTargetIndex >= static_cast<int>(addableTargets.size())) {
                        addTargetIndex = 0;
                    }

                    if (ImGui::BeginCombo("添加分类", GetRuleTargetLabel(addableTargets[addTargetIndex]))) {
                        for (int i = 0; i < static_cast<int>(addableTargets.size()); ++i) {
                            const bool selected = i == addTargetIndex;
                            if (ImGui::Selectable(GetRuleTargetLabel(addableTargets[i]), selected)) {
                                addTargetIndex = i;
                            }
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGuiEx::Tooltip("从尚未使用的分类中新增一条规则。");

                    ImGui::SameLine();
                    if (ImGuiEx::Button("添加规则", "把选中的分类加入规则列表，默认模式为“固定上限 1.0x”。")) {
                        rules.push_back(MakeDefaultRule(addableTargets[addTargetIndex]));
                    }
                }
                else {
                    ImGui::TextDisabled("所有分类都已添加。删除现有规则后可重新加入。");
                }

                ImGui::SeparatorText("规则列表");

                if (rules.empty()) {
                    ImGui::TextWrapped("当前没有任何分类规则。此时程序不会额外接管体型上限，所有目标都沿用原系统逻辑。");
                }

                for (size_t i = 0; i < rules.size();) {
                    auto& rule = rules[i];
                    const auto target = GetRuleTarget(rule);
                    const bool supportsActionFit = RuleTargetSupportsActionFit(target);
                    auto mode = GetRuleMode(rule);

                    if (!supportsActionFit && mode == LSizeLimitRuleMode_t::kActionFit) {
                        rule.sMode = EnumName(LSizeLimitRuleMode_t::kSystemAuto);
                        mode = LSizeLimitRuleMode_t::kSystemAuto;
                    }

                    bool deleteRule = false;
                    bool moveUp = false;
                    bool moveDown = false;

                    ImGui::PushID(static_cast<int>(i));

                    ImGui::SeparatorText(fmt::format("规则 {}: {}", i + 1, GetRuleTargetLabel(target)).c_str());
                    ImGuiEx::Tooltip(GetRuleTargetTooltip(target));

                    ImGui::Checkbox("启用", &rule.bEnabled);

                    ImGui::SameLine();
                    ImGui::BeginDisabled(i == 0);
                    if (ImGui::ArrowButton("##MoveUp", ImGuiDir_Up)) {
                        moveUp = true;
                    }
                    ImGui::EndDisabled();

                    ImGui::SameLine();
                    ImGui::BeginDisabled(i + 1 >= rules.size());
                    if (ImGui::ArrowButton("##MoveDown", ImGuiDir_Down)) {
                        moveDown = true;
                    }
                    ImGui::EndDisabled();

                    ImGui::SameLine();
                    if (ImGuiEx::Button("删除", "删除这条分类规则。")) {
                        deleteRule = true;
                    }

                    ImGui::TextWrapped("%s", GetRuleTargetTooltip(target));

                    if (ImGui::BeginCombo("模式", GetRuleModeLabel(mode))) {
                        for (int rawMode = 0; rawMode < static_cast<int>(LSizeLimitRuleMode_t::kTotal); ++rawMode) {
                            const auto candidate = static_cast<LSizeLimitRuleMode_t>(rawMode);
                            if (candidate == LSizeLimitRuleMode_t::kActionFit && !supportsActionFit) {
                                continue;
                            }

                            const bool selected = candidate == mode;
                            if (ImGui::Selectable(GetRuleModeLabel(candidate), selected)) {
                                rule.sMode = EnumName(candidate);
                                mode = candidate;
                            }
                            if (selected) {
                                ImGui::SetItemDefaultFocus();
                            }
                        }
                        ImGui::EndCombo();
                    }
                    ImGuiEx::Tooltip(GetRuleModeTooltip(mode));

                    if (mode == LSizeLimitRuleMode_t::kFixedLimit) {
                        float value = std::clamp(rule.fValue, SizeRuleFixedLimitMin, SizeRuleFixedLimitMax);
                        ImGui::SetNextItemWidth(ImGui::GetFontSize() * 8.0f);
                        if (ImGui::InputFloat("上限", &value, 0.05f, 0.50f, "%.2fx", ImGuiInputTextFlags_AutoSelectAll)) {
                            rule.fValue = std::clamp(value, SizeRuleFixedLimitMin, SizeRuleFixedLimitMax);
                        }
                        ImGui::SameLine();
                        ImGui::TextDisabled("固定值是绝对体型，不是相对自然体型的倍率。");
                    }
                    else if (mode == LSizeLimitRuleMode_t::kActionFit) {
                        ImGui::TextDisabled("当前动作适应结果：%.2fx", GetActionCompatibleSizeLimit());
                    }
                    else if (mode == LSizeLimitRuleMode_t::kNaturalCeiling) {
                        ImGui::TextDisabled("只封顶，不会把已经缩小的目标拉回去。");
                    }
                    else if (mode == LSizeLimitRuleMode_t::kNaturalLock) {
                        ImGui::TextDisabled("会把被缩小的目标也逐步拉回自然体型。");
                    }
                    else if (mode == LSizeLimitRuleMode_t::kSystemAuto) {
                        ImGui::TextDisabled("命中后停止继续匹配，并交回该目标自己的原系统成长逻辑。");
                    }
                    else if (mode == LSizeLimitRuleMode_t::kUnlimited) {
                        ImGui::TextDisabled("命中后取消额外上限。");
                    }

                    ImGui::PopID();

                    if (deleteRule) {
                        rules.erase(rules.begin() + static_cast<std::ptrdiff_t>(i));
                        continue;
                    }

                    if (moveUp && i > 0) {
                        std::swap(rules[i], rules[i - 1]);
                        --i;
                        continue;
                    }

                    if (moveDown && i + 1 < rules.size()) {
                        std::swap(rules[i], rules[i + 1]);
                        ++i;
                        continue;
                    }

                    ++i;
                }

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
