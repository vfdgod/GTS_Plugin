#include "UI/Controls/StatusBar.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Core/ImUtil.hpp"
#include "Managers/AttributeManager.hpp"

namespace {

	PSString TDamageResist = "总伤害抗性百分比。\n"
		"实际提高总生命值。";

	PSString TOnTheEdge = "生命低于 60%% 时：\n"
		"- 当前生命越低，获得的所有成长越强。\n"
		"- 当前生命越低，敌对缩小效果越弱。\n\n"
		"生命剩余 10%% 或更低时效果最强。";

	PSString TSizeReserve = "体型储备 Perk 储存的总体型。\n"
		"通过吞噬、吸收或碾压他人获得。";

	PSString TAspectOfGTS = "这是“女巨人化身”附魔的强度。\n"
		"女巨人化身会影响：\n"
		"- 最大体型、通用缩小强度和体型偷取法术强度\n"
		"- 体型相关伤害，以及任务/平衡模式中的最低缩小阈值\n"
		"- 随机成长的成长速率和成长几率\n"
		"- 缩小爆发强度，以及受击获得的成长量\n"
		"- 对所有敌对缩小来源的缩小抗性\n\n"
		"可从“巨人护符”获得该附魔；它会随机出现在地牢尽头的宝箱中。";

	PSString TLifeAbsorb = "当前生命吸收层数。\n"
				           "拥有对应 Perk 并执行体型相关过量击杀后获得。";


	PSString TCataclismicStompStacks = "当前灾变踩踏层数。\n"
		                               "拥有对应 Perk，并在吞噬完全消化目标后获得。";

	PSString TVoreBeingAbsorbed = "当前正在消化/吸收的小型目标数量";

}

namespace ImGuiEx {

	StatusBar::StatusBar(uint32_t a_iconSize, bool a_enableToolTips): m_iconSize(a_iconSize), m_enableToolTips(a_enableToolTips) {
		//Init Icons
		m_lifeAbsorbIcon            = std::make_unique<DynIconLifeabsorbStacks>(m_iconSize);
		m_damageReductionIcon       = std::make_unique<DynIconDamageReduction>(m_iconSize);
		m_enchantmentIcon           = std::make_unique<DynIconEnchantment>(m_iconSize);
		m_sizeReserveIcon           = std::make_unique<DynIconSizeReserve>(m_iconSize);
		m_onTheEdgeIcon             = std::make_unique<DynIconOnTheEdge>(m_iconSize);
		m_CataclysmicVoreStacksIcon = std::make_unique<DynIconCataclysmicStompStacks>(m_iconSize);
		m_VoreBeingAbsorbedIcon     = std::make_unique<DynIconVoreBeingAbsorbed>(m_iconSize);
	}

	uint8_t StatusBar::Draw(RE::Actor* a_actor, uint32_t a_visFlags, uint32_t a_ASFlags, bool* a_stateChanged, bool a_preview) {

		if (!a_actor) return 0;
		if (!a_actor->Get3D(false)) return 0;

		const auto& P = GTS::Persistent::GetActorData(a_actor);
		const auto& T = GTS::Transient::GetActorData(a_actor);
		if (!T || !P)  return 0;

		// ----- Vars
		float fDamageResist = (1.0f - GTS::AttributeManager::GetAttributeBonus(a_actor, RE::ActorValue::kHealth)) * 100.f;
		float fGTSAspect =  GTS::Ench_Aspect_GetPower(a_actor) * 100.0f;
		float fSizeReserve = P->fSizeReserve;
		float fOnTheEdge = (GTS::GetPerkBonus_OnTheEdge(a_actor, 0.01f) - 1.0f) * 100.f;
		int iLifeAbsorbStacks = T->Stacks_Perk_LifeForce;
		int iVoreStacks = T->Stacks_Perk_CataclysmicStomp;
		int iVoreAbsorbing = T->VoreCurrentlyAbsorbingCount;

		// Dummy values for preview mode
		if (a_preview) {
			fDamageResist = 100.0f;
			fGTSAspect = 100.f;
			fSizeReserve = 2.5f;
			fOnTheEdge = 100.0f;
			iLifeAbsorbStacks = 10;
			iVoreStacks = 10;
			iVoreAbsorbing = 4;
		}

		bool Draw_damageReductionIcon       = !(a_visFlags & StatusbarFlag_HideDamageReduction);
		bool Draw_lifeAbsorbIcon            = !(a_visFlags & StatusbarFlag_HideLifeAbsorbtion);
		bool Draw_enchantmentIcon           = !(a_visFlags & StatusbarFlag_HideEnchantment);
		bool Draw_CataclysmicVoreStacksIcon = !(a_visFlags & StatusbarFlag_HideVoreStacks);
		bool Draw_sizeReserveIcon           = !(a_visFlags & StatusbarFlag_HideSizeReserve);
		bool Draw_onTheEdgeIcon             = !(a_visFlags & StatusbarFlag_HideOnTheEdge);
		bool Draw_voreBeingAbsorbed         = !(a_visFlags & StatusbarFlag_HideVoreBeingAbsorbed);

		bool AS_damageReductionIcon       = (a_ASFlags & StatusbarASFlag_ASDamageReduction);
		bool AS_lifeAbsorbIcon            = (a_ASFlags & StatusbarASFlag_ASLifeAbsorbtion);
		bool AS_enchantmentIcon           = (a_ASFlags & StatusbarASFlag_ASEnchantment);
		bool AS_CataclysmicVoreStacksIcon = (a_ASFlags & StatusbarASFlag_ASVoreStacks);
		bool AS_sizeReserveIcon           = (a_ASFlags & StatusbarASFlag_ASSizeReserve);
		bool AS_onTheEdgeIcon             = (a_ASFlags & StatusbarASFlag_ASOnTheEdge);
		bool AS_voreBeingAbsorbed         = (a_ASFlags & StatusbarASFlag_ASVoreBeingAbsorbed);

		const float RowY = ImGui::GetCursorPosY();
		float cursorX = ImGui::GetCursorPosX();
		uint8_t drawnIcons = 0;
		float m_combiCurValueState = 0.0f;

		//m_damageReductionIcon
		if (Draw_damageReductionIcon) {
			m_damageReductionIcon->Resize(m_iconSize);
			if (m_damageReductionIcon->Draw(fDamageResist, AS_damageReductionIcon)) {
				if (m_enableToolTips) Tooltip(TDamageResist, true);
				ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
				m_combiCurValueState += fDamageResist;
				drawnIcons++;
			}
		}

		//m_lifeAbsorbIcon
		if (Draw_lifeAbsorbIcon) {
			m_lifeAbsorbIcon->Resize(m_iconSize);
			if (m_lifeAbsorbIcon->Draw(iLifeAbsorbStacks, AS_lifeAbsorbIcon)) {
				if (m_enableToolTips) Tooltip(TLifeAbsorb, true);
				ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
				m_combiCurValueState += iLifeAbsorbStacks;
				drawnIcons++;
			}
		}

		//m_enchantmentIcon
		if (Draw_enchantmentIcon) {
			m_enchantmentIcon->Resize(m_iconSize);
			if (m_enchantmentIcon->Draw(fGTSAspect, AS_enchantmentIcon)) {
				if (m_enableToolTips) Tooltip(TAspectOfGTS, true);
				ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
				m_combiCurValueState += fGTSAspect;
				drawnIcons++;
			}
		}

		//m_CataclysmicVoreStacksIcon
		if (Draw_CataclysmicVoreStacksIcon) {
			m_CataclysmicVoreStacksIcon->Resize(m_iconSize);
			if (m_CataclysmicVoreStacksIcon->Draw(iVoreStacks, AS_CataclysmicVoreStacksIcon)) {
				if (m_enableToolTips) Tooltip(TCataclismicStompStacks, true);
				ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
				m_combiCurValueState += iVoreStacks;
				drawnIcons++;
			}
		}

		//m_VoreBeingAbsorbedIcon
		if (Draw_voreBeingAbsorbed) {
			m_VoreBeingAbsorbedIcon->Resize(m_iconSize);
			if (m_VoreBeingAbsorbedIcon->Draw(iVoreAbsorbing, AS_voreBeingAbsorbed)) {
				if (m_enableToolTips) Tooltip(TVoreBeingAbsorbed, true);
				ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
				m_combiCurValueState += iVoreAbsorbing;
				drawnIcons++;
			}
		}

		// Player Only
		if (a_actor->IsPlayerRef()) {

			//m_sizeReserveIcon
			if (Draw_sizeReserveIcon) {
				m_sizeReserveIcon->Resize(m_iconSize);
				if (m_sizeReserveIcon->Draw(fSizeReserve, AS_sizeReserveIcon)) {
					if (m_enableToolTips) Tooltip(TSizeReserve, true);
					ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
					m_combiCurValueState += fSizeReserve;
					drawnIcons++;
				}
			}

			//m_onTheEdgeIcon
			if (Draw_onTheEdgeIcon) {
				m_onTheEdgeIcon->Resize(m_iconSize);
				if (m_onTheEdgeIcon->Draw(fOnTheEdge, AS_onTheEdgeIcon)) {
					if (m_enableToolTips) Tooltip(TOnTheEdge, true);
					m_combiCurValueState += fOnTheEdge;
					drawnIcons++;
				}
				ImUtil::AdvanceCursorInline(cursorX, RowY, m_iconSize, m_padding);
			}

		}

		if (std::abs(m_combiCurValueState - m_combiLastValueState) >= 0.01f) {
			m_combiLastValueState = m_combiCurValueState;
			if (a_stateChanged) *a_stateChanged = true;
		}
		else {
			if (a_stateChanged) *a_stateChanged = false;
		}

		return drawnIcons;

	}
}
