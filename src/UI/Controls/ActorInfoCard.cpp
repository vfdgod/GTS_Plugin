#include "UI/Controls/ActorInfoCard.hpp"

#include "Config/Config.hpp"

#include "UI/Core/ImColorUtils.hpp"
#include "UI/Core/ImFontManager.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Controls/ProgressBar.hpp"

#include "Managers/SpectatorManager.hpp"
#include "Managers/MaxSizeManager.hpp"
#include "Managers/AttributeManager.hpp"

#include "Utils/KillDataUtils.hpp"

namespace {

	// ------- Tooltips -------

	PSString TDamageBonus = "非体型相关的伤害倍率。影响物理伤害和魔法伤害。";
	PSString THHDamage = "穿高跟鞋时额外的脚部伤害倍率。";
	PSString TShrinkResist = "缩小抗性会降低任何缩小法术和/或效果的有效性。";
	PSString TAbsorbedAttributesCap = "已吸收属性不能超过此数值。";

	PSString TStoredAttributes = "储存属性是尚未吸收的永久生命、魔法或耐力提升。\n"
						         "它们会随机分配到三项主要属性中。\n"
						         "满足 Perk 要求后才能充分利用，并将其转换为已吸收属性。";

	PSString TAbsorbedAttributes = "已吸收属性会永久提高生命、魔法或耐力。";

}

namespace ImGuiEx {

	using namespace GTS;

	std::optional<ActorInfoCard::ActorInfo> ActorInfoCard::ActorInfo::GetData(Actor* a_actor) {

		auto I = ActorInfo{};

		if (!a_actor) return std::nullopt;
		if (!a_actor->Get3D(false)) return std::nullopt;

		const auto& P = Persistent::GetActorData(a_actor);
		const auto& T = Transient::GetActorData(a_actor);
		//const auto& K = Persistent::GetKillCountData(a_actor);
		if (!T || !P)  return std::nullopt;

		I.pTargetActor = a_actor->CreateRefHandle();
		I.Name         = a_actor->GetName();

		//Scale
		//I.fScaleNatural     = get_natural_scale(a_actor, true);
		I.fScaleCurrent     = get_visual_scale(a_actor);
		I.fScaleMax         = get_max_scale(a_actor);
		I.fMassModeScaleMax = MassMode_GetValuesForMenu(a_actor);
		I.fScaleProgress    = I.fScaleMax < 2500.0f ? I.fScaleCurrent / I.fScaleMax : -1.0f;

		//Bonuses
		I.fScaleBonus          = T->PotionMaxSize;
		I.fShrinkResistance    = (1.0f - 1.0f * Potion_GetShrinkResistance(a_actor) * Perk_GetSprintShrinkReduction(a_actor)) * 100.f;
		I.fHighHeelDamageBonus = (GetHighHeelsBonusDamage(a_actor, true) - 1.0f) * 100.0f;
		I.fGTSAspect           = Ench_Aspect_GetPower(a_actor) * 100.0f;

		//Stats
		I.fSpeedMult   = (AttributeManager::GetAttributeBonus(a_actor, ActorValue::kSpeedMult) - 1.0f) * 100.f;
		I.fJumpMult    = (AttributeManager::GetAttributeBonus(a_actor, ActorValue::kJumpingBonus) - 1.0f) * 100.0f;
		I.fDamageBonus = (AttributeManager::GetAttributeBonus(a_actor, ActorValue::kAttackDamageMult) - 1.0f) * 100.0f;

		//Stolen Attributes
		I.fStolenAtributes = P->fStolenAttibutes;
		I.fStolenHealth    = GetStolenAttributes_Values(a_actor, ActorValue::kHealth);
		I.fStolenMagicka   = GetStolenAttributes_Values(a_actor, ActorValue::kMagicka);
		I.fStolenStamina   = GetStolenAttributes_Values(a_actor, ActorValue::kStamina);
		I.fStolenCap       = GetStolenAttributeCap(a_actor);

		//Other
		I.iTotalKills    = GetKillCount(a_actor, DeathType::kTotalKills);
		I.bIsPlayer      = a_actor->IsPlayerRef();
		I.fSizeEssence   = P->fExtraPotionMaxScale;
		I.fSkillLevel    = GetGtsSkillLevel(a_actor);
		I.fSkillRatio    = !I.bIsPlayer ? P->fGTSSkillRatio : Runtime::GetFloat(Runtime::GLOB.GTSSkillRatio);

		//OverKills
		I.fOverkillsExtraSize = T->CollossalGrowthSizeBonus;
		I.fCollosalGrowthMult = T->OverkillSizeBonus;

		//Formated
		I.sFmtScale  = fmt::format("{} [{:.2f}x]", GetFormatedHeight(a_actor), I.fScaleCurrent);
		I.sFmtWeight = GetFormatedWeight(a_actor).c_str();

		//Perk Check
		I.bHasPerk_GTSFullAssimilation = Runtime::HasPerk(a_actor, Runtime::PERK.GTSPerkFullAssimilation);
		I.bHasPerk_GTSCollosalGrowth = Runtime::HasPerk(a_actor, Runtime::PERK.GTSPerkColossalGrowth);
		I.bHasPerk_GTSOverindulgence = Runtime::HasPerk(a_actor, Runtime::PERK.GTSPerkOverindulgence);

		return I;

	}

	ActorInfoCard::ActorInfoCard(): m_expandedSec(Section::kSectionExtra) {

		m_wChildFlags = ImGuiChildFlags_Borders           |
						ImGuiChildFlags_NavFlattened      |
						ImGuiChildFlags_AutoResizeY       |
						ImGuiChildFlags_AlwaysAutoResize;

		m_wWindowFlags = ImGuiWindowFlags_NoSavedSettings;


		m_buffs = std::make_unique<StatusBar>(m_buffIconSize, true);


	}

	void ActorInfoCard::Draw(Actor* a_actor, const ImVec2& card_size) {

		if (!a_actor) return;
		if (!a_actor->Get3D1(false)) return;
		const auto& Style = ImGui::GetStyle();

		auto Data = ActorInfo::GetData(a_actor);
		if (!Data.has_value()) return;

		//Update Vars
		bMassModeEnabled = Config::Balance.sSizeMode == "kMassBased";

		ImGui::PushID(a_actor);

		ImGui::BeginChild("##Card", card_size, m_wChildFlags, m_wWindowFlags);

		// Calculate layout
		const float content_width = ImGui::GetContentRegionAvail().x;
		const float button_width = m_baseIconSize + (Style.FramePadding.x * 2.f) + (Style.CellPadding.x * 2.f);

		// Main content area (left side)
		ImGui::BeginChild("##MainContent", { content_width - button_width, 0 }, 
			ImGuiChildFlags_NavFlattened      |
			ImGuiChildFlags_AutoResizeY       |
			ImGuiChildFlags_AlwaysAutoResize,
			m_wWindowFlags | ImGuiWindowFlags_NoBackground
		);
		DrawMainContent(Data.value());

		// Expanded sections
		if (m_expandedSec == Section::kSectionExtra) {
			ImGui::Separator();
			DrawExtraStats(Data.value());
		} else if (m_expandedSec == Section::kSectionAttributes) {
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
			DrawStolenAttributes(Data.value());
		} else if (m_expandedSec == Section::kSectionKillInfo) {
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal);
			DrawKillData(a_actor);
		}

		ImGui::EndChild();

		// Button column (top right)
		ImGui::SameLine();
		ImGui::BeginGroup();

		const bool section1_active = m_expandedSec == Section::kSectionExtra;
		const bool section2_active = m_expandedSec == Section::kSectionKillInfo;
		const bool section3_active = m_expandedSec == Section::kSectionAttributes;

		if (section1_active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImageButton("##ExtraInfo", ImageList::Infocard_ExtraInfo, m_baseIconSize, "显示扩展信息")) {
			m_expandedSec = section1_active ? Section::kNone : Section::kSectionExtra;
		}
		if (section1_active) ImGui::PopStyleColor();
		
		if (section2_active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImageButton("##KillInfo", ImageList::Infocard_Kills, m_baseIconSize, "显示击杀统计信息")) {
			m_expandedSec = section2_active ? Section::kNone : Section::kSectionKillInfo;
		}
		if (section2_active) ImGui::PopStyleColor();

		if (section3_active) ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
		if (ImageButton("##StatInfo", ImageList::Infocard_StolenStats, m_baseIconSize, "显示窃取属性")) {
			m_expandedSec = section3_active ? Section::kNone : Section::kSectionAttributes;
		}
		if (section3_active) ImGui::PopStyleColor();

		DrawSpectateButton(a_actor);

		ImGui::EndGroup();
		ImGui::EndChild();
		ImGui::PopID();
	}

	void ActorInfoCard::DrawBuffIcons(const ActorInfo& Data) const {

		ImVec2 Size = { ImGui::GetContentRegionAvail().x, m_buffIconSize + (ImGui::GetStyle().FramePadding.y * 2.0f) };

		ImGui::BeginChild("##BuffIcons", Size,
			ImGuiChildFlags_Borders |
			ImGuiChildFlags_FrameStyle |
			ImGuiChildFlags_NavFlattened,
			m_wWindowFlags | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoNav
		);

		uint32_t drawnIcons = m_buffs->Draw(Data.pTargetActor.get().get(), 0, 0, nullptr, false);

		if (drawnIcons == 0) {
			const char* txt = "无激活增益";
			const ImVec2 textSize = ImGui::CalcTextSize(txt);
			ImGui::SetCursorPos({
				Size.x *.5f - (textSize.x * .5f),
				Size.y *.5f - (textSize.y * .5f)
			});
			ImGui::Text(txt);
		}

		ImGui::Dummy({});
		ImGui::EndChild();
	}

	void ActorInfoCard::DrawBaseInfoTable(const ActorInfo& Data) const {

		ImFontManager::Push(ImFontManager::ActiveFontType::kLargeText);

		if (ImGui::BeginTable("##MainStatsTable", 2,
			ImGuiTableFlags_NoSavedSettings |
			ImGuiTableFlags_NoBordersInBody |
			ImGuiTableFlags_Hideable,
			{ ImGui::GetContentRegionAvail().x, 0.0f })) {

			// --------------------------------- Max Size
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("最大体型：");
			ImGui::TableSetColumnIndex(1);

			if (Data.fScaleMax > 2500.f) {
				ImGui::Text("∞");
			}
			else {
				if (bMassModeEnabled) {
					ImGui::Text("%.2fx / %.2fx", Data.fScaleMax, Data.fMassModeScaleMax);
				}
				else {
					ImGui::Text("%.2fx", Data.fScaleMax);
				}
			}

			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("击杀：");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%u", Data.iTotalKills);

			ImGui::EndTable();

		}

		ImFontManager::Pop();

	}

	void ActorInfoCard::DrawMainContent(const ActorInfo& Data) const {

		ImFontManager::Push(ImFontManager::ActiveFontType::kWidgetTitle);
		{
			ImGui::Text(str_toupper(Data.Name).c_str());
			ImGui::SeparatorEx(ImGuiSeparatorFlags_Horizontal, 1.0f);
			{
				ImFontManager::Push(ImFontManager::ActiveFontType::kWidgetBody);

				ProgressBar(
					Data.fScaleProgress == -1.0f ? 1.0f : Data.fScaleProgress,
					{ ImGui::GetContentRegionAvail().x, 0.0f },
					Data.sFmtScale.c_str(),
					ImGuiExProgresbarFlag_Gradient | ImGuiExProgresbarFlag_Rounding,
					//Height, Border Thickness, Rounding, DarkFactor, LightFactor
					1.45f, 0.5f, 5.0f, 0.7f, 1.3f,
					ImUtil::Colors::fRGBToU32(Config::UI.f3AccentColor)
				);

				DrawBuffIcons(Data);
				DrawBaseInfoTable(Data);

			}
		}
		ImFontManager::Pop(2);
	}

	void ActorInfoCard::DrawExtraStats(const ActorInfo& Data) const {


		if (ImGui::BeginTable("##MainStatsTable", 2,
			ImGuiTableFlags_NoSavedSettings |
			ImGuiTableFlags_NoBordersInBody |
			ImGuiTableFlags_Hideable, 
			{ ImGui::GetContentRegionAvail().x, 0.0f })) {

			//---------Total Max Size Calculation and Text Formating
			const float BonusSize_EssenceAndDragons =    Data.fSizeEssence;
			const float BonusSize_TempPotionBoost =      Data.fScaleBonus * 100.0f;
			const float BonusSize_AspectOfGiantess =     Data.fGTSAspect;
			const float BonusSize_Overkills =            Data.fOverkillsExtraSize;
			const float BonusSize_CollosalGrowth =       Data.fCollosalGrowthMult;

			const std::string TotalSizeBonusCalculation = fmt::format(
				fmt::runtime(
					"体型精华与已吸收龙魂：+{:.2f}x\n"
					"巨型成长：x{:.2f}\n"
					"高度药剂：+{:.0f}%%\n"
					"巨人化身：+{:.0f}%%\n"
					"过量击杀：+{:.2f}x\n\n"
					"- 当体型上限设为“技能决定”时，体型精华会提高可达到的最大体型。\n"
					"- 如果体型获取模式设为“质量模式”，精华加成会降低 {:.0f}%%。\n"
					"- 拥有正确 Perk 时，可通过击杀并吸收龙来获得精华。\n"
					"- 也可通过消耗世界各处找到的特定药剂获得。"
				),
				(bMassModeEnabled ? BonusSize_EssenceAndDragons * MassMode_ElixirPowerMultiplier : BonusSize_EssenceAndDragons * 1.0f), //"Size Essence & Absorbed Dragons: +{:.2f}x\n"
				BonusSize_CollosalGrowth,                                                                                               //"Colossal Growth: +{:.2f}x\n"
				BonusSize_TempPotionBoost,                                                                                              //"Potion Of Heights: +{:.0f}%%\n"
				BonusSize_AspectOfGiantess,                                                                                             //"Aspect Of Giantess: +{:.0f}%%\n"
				BonusSize_Overkills,                                                                                                    //"Overkills: +{:.2f}x\n\n"
				(1.0f - MassMode_ElixirPowerMultiplier) * 100.0f
			);

			// --------------------------------- Skill Level
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("技能等级：");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.0f (%.0f%%)", Data.fSkillLevel, (Data.fSkillRatio * 100.f));


			// --------------------------------- Shrink Resistance
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("缩小抗性：");
			ImGuiEx::Tooltip(TShrinkResist, true);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.1f%%", Data.fShrinkResistance);

			// --------------------------------- Weight
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("重量：");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(Data.sFmtWeight.c_str());


			// --------------------------------- Bonus Size
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("额外体型：");
			ImGuiEx::Tooltip(TotalSizeBonusCalculation.c_str(), true);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text(
				bMassModeEnabled ? "%.0f%% + [%.2fx + %.2fx 可能值]" : "%.0f%% + %.2fx + %.2fx",
				(Data.fScaleBonus * 100.0f) + Data.fGTSAspect,
				bMassModeEnabled ? (Data.fSizeEssence * MassMode_ElixirPowerMultiplier) + BonusSize_Overkills : (Data.fSizeEssence * 1.0f) + BonusSize_Overkills,
				std::clamp(BonusSize_CollosalGrowth, 1.0f, 999999.0f)
			);

			// ---------------------------------  High Heel Damage
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("高跟鞋伤害：");
			ImGuiEx::Tooltip(THHDamage, true);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("+%.0f%%", Data.fHighHeelDamageBonus);


			// ---------------------------------  Damage Multiplier
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("额外伤害：");
			ImGuiEx::Tooltip(TDamageBonus, true);
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.1f%%", Data.fDamageBonus);


			// --------------------------------- Carry Weight
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("额外负重：");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.1f", Data.fCarryWeightBonus);


			// --------------------------------- Speed Multiplier
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("额外速度：");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.1f%%", Data.fSpeedMult);


			// ---------------------------------  Jump Multiplier
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::Text("额外跳跃高度：");
			ImGui::TableSetColumnIndex(1);
			ImGui::Text("%.1f%%", Data.fJumpMult);

			ImGui::EndTable();
		}
	}

	void ActorInfoCard::DrawSpectateButton(Actor* a_actor) const {

		if (!a_actor) return;

		const bool isPlayer = a_actor->IsPlayerRef();
		const bool isTargeted = SpectatorManager::GetCameraTarget() == a_actor;
		const bool isCameraOnPlayer = SpectatorManager::IsCameraTargetPlayer();

		// 'X' button appears if this actor is targeted OR if this is the player and camera is NOT on player
		if ((isTargeted && !isPlayer) || (isPlayer && !isCameraOnPlayer)) {
			if (ImageButton("##Spectate", ImageList::Generic_X, m_baseIconSize, "将镜头切回玩家角色。")) {
				SpectatorManager::ResetTarget(true);
			}
		}
		// "Spectate" button appears for non-player actors that are not targeted
		else if (!isPlayer) {
			if (ImageButton("##Spectate", ImageList::Infocard_Spectate, m_baseIconSize, "旁观此 NPC。")) {
				SpectatorManager::SetCameraTarget(a_actor, false);
			}
		}
	}

	void ActorInfoCard::DrawKillStat(Actor* a_actor, const char* a_name, DeathType a_type, uint8_t a_colOffset, const char* a_toolTip) {

		if (!a_actor) return;

		if (a_colOffset == 0) {
			ImGui::TableNextRow();
		}

		ImGui::TableSetColumnIndex(0 + a_colOffset);
		ImGui::Text(a_name);
		if (a_toolTip) {
			ImGuiEx::Tooltip(a_toolTip, true);
		}

		ImGui::TableSetColumnIndex(1 + a_colOffset);
		ImGui::Text("%u", GetKillCount(a_actor, a_type));

	}
	void ActorInfoCard::DrawStolenAttributes(const ActorInfo& Data) {
		if (ImGui::BeginTable("##AttributesTable", 2,
			ImGuiTableFlags_NoSavedSettings |
			ImGuiTableFlags_NoBordersInBody |
			ImGuiTableFlags_Hideable,
			{ ImGui::GetContentRegionAvail().x, 0.0f })) {

			if (Data.bHasPerk_GTSFullAssimilation) {

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("储存属性：");
				ImGuiEx::Tooltip(TStoredAttributes, true);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("+%.2f", Data.fStolenAtributes);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("已吸收属性：");
				ImGuiEx::Tooltip(TAbsorbedAttributes, true);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(" 属性上限：");
				ImGuiEx::Tooltip(TAbsorbedAttributesCap, true);
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.2f", Data.fStolenCap);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(" 生命：");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("+%.2f", Data.fStolenHealth);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(" 魔法：");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("+%.2f", Data.fStolenMagicka);

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text(" 耐力：");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("+%.2f", Data.fStolenStamina);

			} else {
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("缺少 Perk：");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("“完全同化”");
			}
			ImGui::EndTable();
		}
	}
	void ActorInfoCard::DrawKillData(Actor* a_actor) {

		if (!a_actor) return;

		ImGui::Spacing();

		if (ImGui::BeginTable("##KillCountTable", 4, 
			ImGuiTableFlags_NoSavedSettings | 
			ImGuiTableFlags_NoBordersInBody | 
			ImGuiTableFlags_Hideable, 
			{ ImGui::GetContentRegionAvail().x, 0.0f })) {

			//The Row Draws Must be interleaved, as we have 4 columns, a_colOffset > 0 Prevents moving to then next row.

			DrawKillStat(a_actor, "抹除存在：",             DeathType::kErasedFromExistence, 0 );
			DrawKillStat(a_actor, "手指碾压：",             DeathType::kFingerCrushed,       2 );
			DrawKillStat(a_actor, "缩小到消失：",           DeathType::kShrunkToNothing ,    0 );
			DrawKillStat(a_actor, "抓取碾压：",             DeathType::kGrabCrushed,         2 );
			DrawKillStat(a_actor, "胸部窒息：",             DeathType::kBreastSuffocated,    0 );
			DrawKillStat(a_actor, "臀部碾压：",             DeathType::kButtCrushed,         2 );
			DrawKillStat(a_actor, "胸部吸收：",             DeathType::kBreastAbsorbed,      0 );
			DrawKillStat(a_actor, "拥抱碾压：",             DeathType::kHugCrushed,          2 );
			DrawKillStat(a_actor, "胸部碾压：",             DeathType::kBreastCrushed,       0 );
			DrawKillStat(a_actor, "碾压：",                 DeathType::kCrushed,             2 );
			DrawKillStat(a_actor, "大腿窒息：",             DeathType::kThighSuffocated,     0 );
			DrawKillStat(a_actor, "吞食：",                 DeathType::kEaten,               2 );
			DrawKillStat(a_actor, "大腿夹击：",             DeathType::kThighSandwiched,     0 );
			DrawKillStat(a_actor, "其他来源：",             DeathType::kOtherSources,        2,
				"其他来源包括：\n"
				"- 冲击波死亡\n"
				"- Tiny Calamity 碰撞死亡\n"
				"- 被抓住且无动作发生时的小体型死亡\n"
				"- 大体型时的过量武器伤害\n"
			);
			DrawKillStat(a_actor, "大腿碾压：",             DeathType::kThighCrushed);
			DrawKillStat(a_actor, "碾磨碾压：",             DeathType::kGrinded);
			DrawKillStat(a_actor, "踢击碾压：",             DeathType::kKicked);

			ImGui::EndTable();
		}
	}
}
