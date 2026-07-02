#include "UI/Windows/Settings/Categories/Extensions.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/ConditionalHeader.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/Text.hpp"
#include "UI/Controls/ToolTip.hpp"

#include "Config/Config.hpp"
#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Managers/MaxSizeManager.hpp"

#include <algorithm>

namespace GTS {

	namespace {
		constexpr float SizeRuleFixedLimitMin = 0.05f;
		constexpr float SizeRuleFixedLimitMax = 255.0f;

		template <class Enum>
		std::string EnumName(Enum a_value) {
			return std::string(magic_enum::enum_name(a_value));
		}

		const char* GetShrinkModeLabel(SettingsAdvanced_t::LShrinkApplicationMode_t a_mode) {
			switch (a_mode) {
				case SettingsAdvanced_t::LShrinkApplicationMode_t::kGradual:
					return "逐渐缩小";
				case SettingsAdvanced_t::LShrinkApplicationMode_t::kInstant:
					return "瞬间缩小";
				default:
					return "未知模式";
			}
		}

		const char* GetShrinkModeTooltip(SettingsAdvanced_t::LShrinkApplicationMode_t a_mode) {
			switch (a_mode) {
				case SettingsAdvanced_t::LShrinkApplicationMode_t::kGradual:
					return "尺寸降低时，继续沿用当前 half-life 平滑过渡到目标体型。\n"
					       "适用于法术、药水、伤害、游戏模式，以及超过体型上限后的系统回拉。";
				case SettingsAdvanced_t::LShrinkApplicationMode_t::kInstant:
					return "尺寸降低时，直接把当前显示体型同步到新的目标体型，不再等待平滑收缩。\n"
					       "适用于法术、药水、伤害、游戏模式，以及超过体型上限后的系统回拉。\n"
					       "只影响“变小”方向；变大仍按原逻辑处理。";
				default:
					return "";
			}
		}

		bool TryParseShrinkMode(const std::string& a_value, SettingsAdvanced_t::LShrinkApplicationMode_t& a_out) {
			if (auto parsed = magic_enum::enum_cast<SettingsAdvanced_t::LShrinkApplicationMode_t>(a_value); parsed.has_value()) {
				a_out = *parsed;
				return true;
			}
			return false;
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

		bool RuleTargetSupportsActionFit(LSizeLimitRuleTarget_t a_target) {
			return a_target != LSizeLimitRuleTarget_t::kPlayer;
		}

		bool RuleTargetSupportsCombatOnly(LSizeLimitRuleTarget_t a_target) {
			return a_target == LSizeLimitRuleTarget_t::kHostile;
		}

		const char* GetRuleCombatOnlyTooltip(LSizeLimitRuleTarget_t a_target) {
			switch (a_target) {
				case LSizeLimitRuleTarget_t::kHostile:
					return "开启后，只有当前处于战斗中的敌对目标才会命中这条规则。未开战目标会继续按后续分类匹配。";
				default:
					return "";
			}
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
	}

	CategoryExtensions::CategoryExtensions() {
		m_name = "扩展";
	}

	void CategoryExtensions::DrawLeft() {

		ImUtil_Unique
		{
			PSString T0 = "开启后，当前存档会优先读取并写入 Data/SKSE/Plugins/GTSPlugin/SharedSettings.toml，\n"
			              "用于共享除角色/世界进度外的模组配置。\n"
			              "关闭后不会恢复旧的存档配置，只是从下一次保存开始改回按存档保存。";

			if (ImGui::CollapsingHeader("配置共享", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("全局共享配置", &Config::Advanced.bShareSettingsGlobally, T0);
				ImGui::Spacing();
			}
		}

		ImUtil_Unique
		{
			PSString T0 = "开启后，玩家会被视为微型灾厄正在生效。\n"
			              "这会影响所有检查 TinyCalamityActive() 的逻辑，例如禁用随机成长、启用持续缩小逻辑、狂怒灾厄前置等。\n"
			              "不会播放龙吼启动音效、爆发特效，也不会自动压制玩家体型。";
			PSString T1 = "开启后，玩家冲刺时启用微型灾厄的蓄力速度、冲撞检测和直接碾碎逻辑。";
			PSString T2 = "开启后，尺寸动作、脚步、踩踏、投掷、抓握、吞噬距离等动作判定按微型灾厄强化规则结算。";
			PSString T3 = "开启后，微型灾厄相关缩小逻辑生效，包括尺寸伤害后的额外缩小和吸收术额外缩小强度。";
			PSString T4 = "开启后，玩家生命抗性、攻击和负重按微型灾厄属性加成规则结算。";
			PSString T5 = "模拟永恒灾厄：微型灾厄击杀/碾碎会延长持续时间，并减少龙吼冷却。";
			PSString T6 = "模拟迫近灾难：冲刺速度蓄力上限更高，微型灾厄碾碎后的速度衰减更轻。";
			PSString T7 = "模拟生命窃取：微型灾厄缩小时获得治疗，并提升额外缩小强度；真实龙吼开始时也会获得更长持续时间。";
			PSString T8 = "模拟狂怒灾厄：微型灾厄激活时，可对低生命值目标触发处决动画。";
			PSString T9 = "模拟缩小凝视：微型灾厄激活时，玩家准星凝视敌人会持续缩小目标。";

			if (ImGui::CollapsingHeader("微型灾厄", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("模拟微型灾厄激活", &Config::Advanced.bPlayerTinyCalamityActive, T0);
				ImGuiEx::CheckBox("冲刺碰撞", &Config::Advanced.bPlayerTinyCalamitySprintBoost, T1);
				ImGui::SameLine();
				ImGuiEx::CheckBox("动作强化", &Config::Advanced.bPlayerTinyCalamityActionBoost, T2);
				ImGuiEx::CheckBox("缩小强化", &Config::Advanced.bPlayerTinyCalamityShrinkBoost, T3);
				ImGui::SameLine();
				ImGuiEx::CheckBox("属性加成", &Config::Advanced.bPlayerTinyCalamityAttributeBoost, T4);
				ImGuiEx::CheckBox("永恒灾厄", &Config::Advanced.bPlayerTinyCalamityRefresh, T5);
				ImGui::SameLine();
				ImGuiEx::CheckBox("迫近灾难", &Config::Advanced.bPlayerTinyCalamityAug, T6);
				ImGuiEx::CheckBox("生命窃取", &Config::Advanced.bPlayerTinyCalamitySizeSteal, T7);
				ImGui::SameLine();
				ImGuiEx::CheckBox("狂怒灾厄", &Config::Advanced.bPlayerTinyCalamityRage, T8, !Config::Advanced.bPlayerTinyCalamityActive);
				ImGuiEx::CheckBox("缩小凝视", &Config::Advanced.bPlayerTinyCalamityShrinkingGaze, T9, !Config::Advanced.bPlayerTinyCalamityActive);

				if (ImGuiEx::Button("清空技能冷却")) {
					CooldownManager::GetSingleton().Reset();
				}

				ImGui::Spacing();
			}
		}

		ImUtil_Unique
		{
			PSString T0 = "决定尺寸变小时，是继续平滑过渡，还是立刻同步到更小的目标体型。\n"
			              "适用于法术、药水、伤害、游戏模式，以及超过体型上限后的系统回拉。";
			SettingsAdvanced_t::LShrinkApplicationMode_t shrinkMode = SettingsAdvanced_t::LShrinkApplicationMode_t::kGradual;
			if (!TryParseShrinkMode(Config::Advanced.sShrinkMode, shrinkMode)) {
				Config::Advanced.sShrinkMode = "kGradual";
			}

			if (ImGui::CollapsingHeader("缩小行为", ImUtil::HeaderFlagsDefaultOpen)) {
				if (ImGui::BeginCombo("缩小方式", GetShrinkModeLabel(shrinkMode))) {
					for (int rawMode = 0; rawMode < static_cast<int>(SettingsAdvanced_t::LShrinkApplicationMode_t::kTotal); ++rawMode) {
						const auto candidate = static_cast<SettingsAdvanced_t::LShrinkApplicationMode_t>(rawMode);
						const bool selected = candidate == shrinkMode;
						if (ImGui::Selectable(GetShrinkModeLabel(candidate), selected)) {
							Config::Advanced.sShrinkMode = EnumName(candidate);
							shrinkMode = candidate;
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGuiEx::Tooltip(T0);
				ImGuiEx::Tooltip(GetShrinkModeTooltip(shrinkMode));

				ImGui::Spacing();
			}
		}

		ImUtil_Unique
		{
			PSString T0 = "启用后，所有非玩家角色在缩小到自然体型以下时，会持续失去当前耐力；\n"
			              "如果该目标本身有最大魔力值，也会一并持续失去当前魔力。\n"
			              "判断基准使用目标自己的自然体型，而不是固定 1.0x。\n"
			              "只要低于自然体型，就会持续被抽取；低于自然体型的 50% 时，会直接被抽到 0。\n"
			              "被抽走的数值会优先回复玩家的对应资源；如果玩家该资源已满，则按 100 点资源 = 1.0 点 GTS 经验折算。\n"
			              "没有魔力值的目标只会处理耐力。";

			if (ImGui::CollapsingHeader("缩小时资源掠夺", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("启用非玩家缩小时抽取耐力/魔力", &Config::Balance.bShrinkStealResources, T0);
				ImGui::Spacing();
			}
		}

		ImUtil_Unique
		{
			PSString T0 = "开启后，玩家的轻踩、重踩、践踏、踢击和爬行挥击会自动选择更靠近目标的一侧。\n"
			              "关闭时保留左右脚/左右手分开的按键控制。\n"
			              "这是作者新增的实验性功能，默认关闭。";

			if (ImGui::CollapsingHeader("脚部 Auto-Aim", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("启用玩家脚部 Auto-Aim", &Config::Advanced.bPlayerFootAutoAim, T0);
				ImGui::Spacing();
			}
		}

		ImUtil_Unique
		{
			PSString T0 = "按下启用的玩家踩踏动作时，把附近足够小的目标预吸附到对应脚下。";
			PSString T1 = "普通轻踩按键是否使用踩踏辅助。";
			PSString T2 = "重踩按键是否使用踩踏辅助。";
			PSString T3 = "践踏 Trample 按键是否使用踩踏辅助。";
			PSString T4 = "开启后，一次踩踏可预吸附多个目标；关闭时只吸附最近目标。";
			PSString T5 = "允许多目标时，一次踩踏最多预吸附多少目标。";
			PSString T6 = "预吸附后阻止目标移动的持续时间。";
			PSString T7 = "搜索目标的基础半径。实际距离会再乘以 1.6 倍和玩家当前体型。";
			PSString T8 = "只有玩家与目标的体型差达到该倍率时，目标才会被视为适合踩踏辅助。\n"
			              "这个值只影响辅助吸附，不等于踩爆阈值；普通踩爆通常还需要约 10x 体型差。";

			if (ImGui::CollapsingHeader("踩踏辅助", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("启用踩踏辅助", &Config::Advanced.bStompAssist, T0);
				ImGuiEx::CheckBox("普通踩踏", &Config::Advanced.bStompAssistNormal, T1, !Config::Advanced.bStompAssist);
				ImGui::SameLine();
				ImGuiEx::CheckBox("重踩", &Config::Advanced.bStompAssistStrong, T2, !Config::Advanced.bStompAssist);
				ImGui::SameLine();
				ImGuiEx::CheckBox("践踏", &Config::Advanced.bStompAssistTrample, T3, !Config::Advanced.bStompAssist);
				ImGuiEx::CheckBox("允许多目标", &Config::Advanced.bStompAssistMultiTarget, T4, !Config::Advanced.bStompAssist);
				ImGuiEx::SliderU8("最大目标数量", &Config::Advanced.iStompAssistMaxTargets, 1, 8, T5, "%d", !Config::Advanced.bStompAssist || !Config::Advanced.bStompAssistMultiTarget);
				ImGuiEx::SliderF("预吸附持续时间", &Config::Advanced.fStompAssistDuration, 0.2f, 2.0f, T6, "%.1f 秒", !Config::Advanced.bStompAssist);
				ImGuiEx::SliderF("目标搜索半径", &Config::Advanced.fStompAssistSearchRadius, 8.0f, 120.0f, T7, "%.1f", !Config::Advanced.bStompAssist);
				ImGuiEx::SliderF("目标体型阈值", &Config::Advanced.fStompAssistSizeThreshold, 1.5f, 20.0f, T8, "%.1fx", !Config::Advanced.bStompAssist);

				ImGui::Spacing();
			}
		}

		ImUtil_Unique
		{
			PSString T0 = "启用后，可对下方选中的脚部动作限制单次尺寸伤害。\n"
			              "限制按目标最大生命值百分比计算，并会考虑技能等级、难度和全局尺寸伤害倍率后的最终伤害。";
			PSString T1 = "限制普通轻踩及其脚下轻踩变体。";
			PSString T2 = "限制重踩及其脚下重踩变体。";
			PSString T3 = "限制践踏 Trample 的阶段伤害。";
			PSString T4 = "单次命中最多造成目标最大生命值的多少百分比。\n"
			              "例如 45% 表示 600 最大生命目标单次最多受到约 270 点最终尺寸伤害。";

			Config::Advanced.fFootActionDamageLimitMaxHealthPercent = std::clamp(
				Config::Advanced.fFootActionDamageLimitMaxHealthPercent,
				1.0f,
				100.0f
			);

			if (ImGui::CollapsingHeader("动作伤害限制", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("启用动作伤害限制", &Config::Advanced.bFootActionDamageLimit, T0);
				ImGuiEx::CheckBox("轻踩", &Config::Advanced.bFootActionDamageLimitNormal, T1, !Config::Advanced.bFootActionDamageLimit);
				ImGui::SameLine();
				ImGuiEx::CheckBox("重踩", &Config::Advanced.bFootActionDamageLimitStrong, T2, !Config::Advanced.bFootActionDamageLimit);
				ImGui::SameLine();
				ImGuiEx::CheckBox("踩踏", &Config::Advanced.bFootActionDamageLimitTrample, T3, !Config::Advanced.bFootActionDamageLimit);
				ImGuiEx::SliderF(
					"单次最大伤害",
					&Config::Advanced.fFootActionDamageLimitMaxHealthPercent,
					1.0f,
					100.0f,
					T4,
					"最大生命 %.0f%%",
					!Config::Advanced.bFootActionDamageLimit
				);

				ImGui::Spacing();
			}
		}
	}

	void CategoryExtensions::DrawRight() {

		ImUtil_Unique
		{
			PSString T0 = "设置“移动缩小目标到附近”快捷键按下后，会扫描玩家周围多大的范围。\n"
			              "只处理当前已加载的角色；范围过大时，一次可能会移动很多目标。";
			PSString T1 = "决定移动过来的目标摆在玩家周围一圈，还是摆在玩家正前方几排。";
			PSString T2 = "移动成功后，让目标短暂无法自行走开。\n"
			              "这是基于短时拦移动实现的软停步，不会改动它们的长期 AI 状态。";
			PSString T3 = "如果切到“自定义分类”，只有下方勾选到的分类才会参与移动。\n"
			              "无论哪种模式，都只会移动当前已经缩小到自然体型以下的非玩家目标。";
			PSString T4 = "设置移动后，最近一圈/第一排目标与玩家之间的距离。";
			PSString T5 = "设置移动后，缩小目标之间的间距。\n"
			              "环形模式会影响同圈相邻目标和圈层距离；面前模式会影响左右间距和前后排距离。";
			PSString T6 = "启用后，会按下方间隔自动执行一次“移动缩小目标到附近”。";
			PSString T7 = "设置自动移动的执行间隔。";
			PSString T8 = "启用后，会按下方间隔提醒附近是否存在符合筛选条件的缩小目标。";
			PSString T9 = "设置自动提醒的检查间隔。";
			LShrinkRecallFilterMode_t filterMode = LShrinkRecallFilterMode_t::kAllShrunken;
			if (!TryParseRecallFilterMode(Config::Balance.sShrinkRecallFilterMode, filterMode)) {
				Config::Balance.sShrinkRecallFilterMode = EnumName(filterMode);
			}

			LShrinkRecallPlacement_t placementMode = LShrinkRecallPlacement_t::kRing;
			if (!TryParseRecallPlacement(Config::Balance.sShrinkRecallPlacement, placementMode)) {
				Config::Balance.sShrinkRecallPlacement = EnumName(placementMode);
			}

			Config::Balance.fShrinkRecallSearchRadius = std::clamp(Config::Balance.fShrinkRecallSearchRadius, 250.0f, 20000.0f);
			Config::Balance.fShrinkRecallPlayerDistance = std::clamp(Config::Balance.fShrinkRecallPlayerDistance, 60.0f, 2000.0f);
			Config::Balance.fShrinkRecallActorSpacing = std::clamp(Config::Balance.fShrinkRecallActorSpacing, 40.0f, 1000.0f);
			Config::Balance.fShrinkRecallPauseDuration = std::clamp(Config::Balance.fShrinkRecallPauseDuration, 0.0f, 10.0f);
			Config::Balance.fShrinkRecallAutoInterval = std::clamp(Config::Balance.fShrinkRecallAutoInterval, 1.0f, 600.0f);
			Config::Balance.fShrinkRecallNotifyInterval = std::clamp(Config::Balance.fShrinkRecallNotifyInterval, 1.0f, 600.0f);

			if (ImGui::CollapsingHeader("移动缩小目标到附近", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::HelpText("快捷键说明", "快捷键本身在“按键绑定 -> 能力 / Perk”里设置，默认是 NUMPAD0。");

				if (ImGui::BeginCombo("移动筛选", GetRecallFilterModeLabel(filterMode))) {
					for (int rawMode = 0; rawMode < static_cast<int>(LShrinkRecallFilterMode_t::kTotal); ++rawMode) {
						const auto candidate = static_cast<LShrinkRecallFilterMode_t>(rawMode);
						const bool selected = candidate == filterMode;
						if (ImGui::Selectable(GetRecallFilterModeLabel(candidate), selected)) {
							Config::Balance.sShrinkRecallFilterMode = EnumName(candidate);
							filterMode = candidate;
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGuiEx::Tooltip(GetRecallFilterModeTooltip(filterMode));

				ImGuiEx::SliderF("搜索范围", &Config::Balance.fShrinkRecallSearchRadius, 250.0f, 20000.0f, T0, "%.0f");
				ImGuiEx::SliderF("距玩家距离", &Config::Balance.fShrinkRecallPlayerDistance, 60.0f, 2000.0f, T4, "%.0f");
				ImGuiEx::SliderF("目标间距", &Config::Balance.fShrinkRecallActorSpacing, 40.0f, 1000.0f, T5, "%.0f");

				if (ImGui::BeginCombo("落点模式", GetRecallPlacementLabel(placementMode))) {
					for (int rawMode = 0; rawMode < static_cast<int>(LShrinkRecallPlacement_t::kTotal); ++rawMode) {
						const auto candidate = static_cast<LShrinkRecallPlacement_t>(rawMode);
						const bool selected = candidate == placementMode;
						if (ImGui::Selectable(GetRecallPlacementLabel(candidate), selected)) {
							Config::Balance.sShrinkRecallPlacement = EnumName(candidate);
							placementMode = candidate;
						}
						if (selected) {
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}
				ImGuiEx::Tooltip(T1);
				ImGuiEx::Tooltip(GetRecallPlacementTooltip(placementMode));

				ImGuiEx::SliderF("移动后停步时长", &Config::Balance.fShrinkRecallPauseDuration, 0.0f, 10.0f, T2, "%.1f 秒");
				ImGuiEx::CheckBox("自动移动缩小目标", &Config::Balance.bShrinkRecallAuto, T6);
				ImGuiEx::SliderF("自动移动间隔", &Config::Balance.fShrinkRecallAutoInterval, 1.0f, 600.0f, T7, "每 %.0f 秒", !Config::Balance.bShrinkRecallAuto);
				ImGuiEx::CheckBox("自动提醒附近缩小目标", &Config::Balance.bShrinkRecallNotifyNearby, T8);
				ImGuiEx::SliderF("自动提醒间隔", &Config::Balance.fShrinkRecallNotifyInterval, 1.0f, 600.0f, T9, "每 %.0f 秒", !Config::Balance.bShrinkRecallNotifyNearby);

				if (filterMode == LShrinkRecallFilterMode_t::kCustomTargets) {
					ImGui::SeparatorText("自定义分类");
					ImGuiEx::HelpText("筛选规则", T3);

					for (int rawTarget = 0; rawTarget < static_cast<int>(LSizeLimitRuleTarget_t::kTotal); ++rawTarget) {
						const auto target = static_cast<LSizeLimitRuleTarget_t>(rawTarget);
						if (target == LSizeLimitRuleTarget_t::kPlayer) {
							continue;
						}

						bool selected = IsRecallTargetSelected(Config::Balance.ShrinkRecallTargets, target);
						if (ImGui::Checkbox(GetRuleTargetLabel(target), &selected)) {
							SetRecallTargetSelected(Config::Balance.ShrinkRecallTargets, target, selected);
						}
						ImGuiEx::Tooltip(GetRuleTargetTooltip(target));
						if (target == LSizeLimitRuleTarget_t::kFollower || target == LSizeLimitRuleTarget_t::kImportant || target == LSizeLimitRuleTarget_t::kDragon) {
							ImGui::SameLine();
						}
					}
				}

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

			if (ImGuiEx::ConditionalHeader("体型上限规则", DisableReason, HasPerk && Unlock && !Config::Balance.bBalanceMode)) {
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

					if (RuleTargetSupportsCombatOnly(target)) {
						ImGuiEx::CheckBox("仅限战斗中", &rule.bCombatOnly, GetRuleCombatOnlyTooltip(target));
					}

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
	}
}
