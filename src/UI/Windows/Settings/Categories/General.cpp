#include "UI/Windows/Settings/Categories/General.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Misc.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Core/ImFontManager.hpp"
#include "UI/GTSMenu.hpp"

#include "Config/Config.hpp"
#include "Config/ConfigModHandler.hpp"

#include "Utils/QuestUtil.hpp"
#include "Managers/Animation/AnimationManager.hpp"

#include "UI/Controls/Text.hpp"

namespace {

	PSString T_Export = "将当前模组配置导出到文件。";
	PSString T_Import = "从选中的导出文件导入当前模组配置。";
	PSString T_Delete = "删除当前选中的设置导出。";
	PSString T_Cleanup = "删除除最近 5 个以外的所有导出。";

	PSString T_Reset = "重置所有设置，但不会重置已修改的动作按键绑定。\n"
	                   "如需重置按键绑定，请在对应页面点击重置按钮。";

	PSString T_PandoraWarn = "本模组动画是按 Nemesis 行为生成器设计的。\n"
						     "Pandora 虽然能使用 Nemesis 的行为格式，但并不完全兼容。\n"
		                     "如果使用 Pandora 为本模组生成行为，可能会遇到细微动画错误、不同步，或部分动画完全无法播放。";

	PSString T_FirstPersonWarn = "无法检查第一人称动画，因为这些动画没有第一人称版本。\n"
							     "请切换到第三人称，或使用 Improved Camera 2 的 “Fake First Person”，\n"
			                     "以便在第一人称中使用第三人称动画。";

	PSString T_UnknownBehavior = "无法确定使用的行为生成器。\n"
							     "这可能表示你没有运行 Nemesis/Pandora，或使用了其他工具生成行为。";

	void DrawImportExport(){

		static std::string statusText = "";
		static int selectedExportIndex = -1;
		constexpr int keepCount = 5;

		// Get available export files
		auto exportFiles = GTS::Config::GetExportedFiles();
		std::vector<std::string> fileNames;
		for (const auto& file : exportFiles) {
			fileNames.push_back(file.filename().string());
		}

		// Export section
		if (ImGuiEx::ImageButton("##Export", ImageList::Export_Save, 32, T_Export)) {
			if (GTS::Config::ExportSettings()) {
				auto files = GTS::Config::GetExportedFiles();
				if (!files.empty()) {
					statusText = fmt::format("✓ 已保存为 {}", files.front().filename().string());
				}
			}
		}

		ImGui::SameLine();

		if (ImGuiEx::ImageButton("##Import", ImageList::Export_Load, 32, T_Import)) {
			if(selectedExportIndex < 0) {
				statusText = "请先选择一个导出文件";
			}
			else if (selectedExportIndex >= 0 && selectedExportIndex < exportFiles.size()) {
				if (GTS::Config::LoadFromExport(exportFiles[selectedExportIndex].string())) {
					GTS::EventDispatcher::DoConfigRefreshEvent();
					statusText = fmt::format("✓ 已应用 {}", fileNames[selectedExportIndex]);
				}
				else {
					statusText = "导入失败";
				}
			}
		}

		ImGui::SameLine();

		if (ImGuiEx::ImageButton("##Delete", ImageList::Export_Delete, 32, T_Delete)) {

			if (selectedExportIndex < 0) {
				statusText = "请先选择一个导出文件";
			}
			else if (selectedExportIndex >= 0 && selectedExportIndex < exportFiles.size()) {
				GTS::Config::DeleteExport(exportFiles[selectedExportIndex].string());
				statusText = fmt::format("已删除 {}", fileNames[selectedExportIndex]);
				selectedExportIndex = -1;
			}
		}

		ImGui::SameLine();

		if (ImGuiEx::ImageButton("##Cleaunup", ImageList::Export_Cleanup, 32, T_Cleanup)) {
			GTS::Config::CleanOldExports(keepCount);
			statusText = fmt::format("✓ 已移除旧导出 [保留最近 {0} 个]", keepCount);
		}

		ImGuiEx::SeperatorV();

		if (ImGuiEx::ImageButton("##Reset", ImageList::Generic_Reset, 32, T_Reset)) {
			GTS::EventDispatcher::DoConfigResetEvent();
			statusText = "✓ 模组设置已重置";
		}

		// Combo box for selecting exports
		const char* previewValue = (
			selectedExportIndex >= 0 && selectedExportIndex < fileNames.size()) ?
			fileNames[selectedExportIndex].c_str() :
			"选择导出文件...";

		if (ImGui::BeginCombo("##ExportCombo", previewValue)) {
			for (int i = 0; i < fileNames.size(); i++) {
				bool isSelected = (selectedExportIndex == i);
				if (ImGui::Selectable(fileNames[i].c_str(), isSelected)) {
					selectedExportIndex = i;
				}
				if (isSelected) {
					ImGui::SetItemDefaultFocus();
				}
			}
			ImGui::EndCombo();
		}

		ImGui::Spacing();

		// Status message
		if (!statusText.empty()) {
			GTS::ImFontManager::Push(GTS::ImFontManager::ActiveFontType::kLargeText);
			ImGui::TextColored(ImUtil::Colors::Warning, statusText.c_str());
			GTS::ImFontManager::Pop();
		}
	}
}

namespace GTS {

	CategoryGeneral::CategoryGeneral() {
		m_name = "通用";
	}

	void CategoryGeneral::DrawLeft() {

		// ----- Animation Check

		ImUtil_Unique 
		{

			PSString T0 = "自动检查有时并不可靠。\n"
						  "点击此按钮可强制尝试播放一次动画。\n"
						  "强烈建议使用时站在地面上并保持不动。\n\n"
						  "随后会弹出消息框，提示动画是否成功播放。";

			if (ImGui::CollapsingHeader("动画检查", ImUtil::HeaderFlagsDefaultOpen)) {
				const auto Player = PlayerCharacter::GetSingleton();
				const bool WorkingAnims = AnimationVars::Utility::BehaviorsInstalled(Player);
				const bool FirstPerson = IsFirstPerson();

				bool IsPandoraGenerated = AnimationVars::Other::IsPandoraGenerated(Player);
				bool IsNemesisGenerated = AnimationVars::Other::IsNemesisGenerated(Player);

				// "Animations Installed" Header
				{
					ImFontManager::Push(ImFontManager::ActiveFontType::kWidgetTitle);

					ImGui::Text("动画已安装：");
					if (FirstPerson) {
						ImGui::SameLine(0);
						ImGui::TextColored(ImUtil::Colors::Warning, "未知");
					}
					else if (WorkingAnims) {
						ImGui::SameLine(0, 1);
						ImGui::TextColored(ImUtil::Colors::OK, "是");
					}
					else {
						ImGui::SameLine(0);
						ImGui::TextColored(ImUtil::Colors::Error, "否");
					}

					ImFontManager::Pop();

					if (FirstPerson) {
						ImGuiEx::Tooltip(T_FirstPersonWarn, true);
					}
				}

				// Behavior Generator Info
				{
					ImGui::Text("行为生成器：");
					if (IsPandoraGenerated) {
						ImGui::SameLine(0);
						ImGui::TextColored(ImUtil::Colors::Warning, "Pandora");
						ImGuiEx::Tooltip(T_PandoraWarn, true);
					}
					else if (IsNemesisGenerated) {
						ImGui::SameLine(0);
						ImGui::TextColored(ImUtil::Colors::OK, "Nemesis");
					}
					else if (FirstPerson) {
						ImGui::SameLine(0);
						ImGui::TextColored(ImUtil::Colors::Warning, "未知");
						ImGuiEx::Tooltip(T_FirstPersonWarn, true);
					}
					else {
						ImGui::SameLine(0);
						ImGui::TextColored(ImUtil::Colors::Error, "无/其他");
						ImGuiEx::Tooltip(T_UnknownBehavior, true);
					}
				}

				if (ImGuiEx::Button("手动测试动画", T0)) {

					//Simulate Closing the menu
					GTSMenu::CloseInputConsumers();

					TaskManager::Run("AnimTestTask", [=](auto& progressData) {

						if (progressData.runtime > 0.2) {

							AnimationManager::StartAnim("StrongStompRight", Player);

							if (progressData.runtime > 0.5) {

								if (AnimationVars::General::IsGTSBusy(Player)) {
									PrintMessageBox("动画应已正常工作。");
								}
								else {
									PrintMessageBox("动画未能开始。");
								}
								return false;
							}
						}
						return true;
					});
				}
				ImGui::Spacing();
			}
		}

		//----------- Settings Export

		ImUtil_Unique 
		{
			if (ImGui::CollapsingHeader("导出/导入设置", ImUtil::HeaderFlagsDefaultOpen)) {
				DrawImportExport();
			}
		}

		//----------- Shortcuts

		ImUtil_Unique 
		{

			PSString T0 = "打开本模组的自定义技能树。";
			PSString T1 = "自动完成本模组任务。";
			PSString T2 = "获得本模组的全部法术。";
			PSString T3 = "立即完成 Perk 树。";
			PSString T4 = "获得本模组的全部龙吼。";

			if (ImGui::CollapsingHeader("快捷操作", ImUtil::HeaderFlagsDefaultOpen)) {
				if (ImGuiEx::Button("打开技能树", T0)) {
					GTSMenu::CloseInputConsumers();
					Runtime::SetFloat(Runtime::GLOB.GTSSkillMenu, 1.0f);
				}

				ImGuiEx::SeperatorV();

				const auto Complete = ProgressionQuestCompleted();

				if (!Complete) {
					if (ImGuiEx::Button("跳过任务", T1)) {
						SkipProgressionQuest();
					}
				}
				else {
					ImGui::SameLine();

					if (ImGuiEx::Button("获得全部法术", T2, !Complete)) {
						GiveAllSpellsToPlayer();
					}

					ImGui::SameLine();

					if (ImGuiEx::Button("获得全部 Perk", T3, !Complete)) {
						GiveAllPerksToPlayer();
					}

					ImGui::SameLine();

					if (ImGuiEx::Button("获得全部龙吼", T4, !Complete)) {
						GiveAllShoutsToPlayer();
					}
				}

				ImGui::Spacing();
			}
		}

		//------ Protect Actors

		ImUtil_Unique 
		{

			PSString T0 = "保护重要 NPC，避免其被碾压、吞噬，或受到体型相关法术/动作影响。";
			PSString T1 = "保护追随者，避免其被碾压、吞噬，或受到体型相关法术/动作影响。";

			if (ImGui::CollapsingHeader("保护角色", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("保护重要 NPC",&Config::General.bProtectEssentials, T0);
				ImGui::SameLine();
				ImGuiEx::CheckBox("保护追随者",&Config::General.bProtectFollowers, T1);
				ImGui::Spacing();
			}
	}
		//------ Compatibility

		ImUtil_Unique 
		{

			PSString T0 = "启用或禁用与 Devourment 模组的实验性兼容。\n"
							 "此兼容开关可能导致角色被延迟吞下（因为 Papyrus 延迟）或其他 Bug。\n\n"
							 "启用后，本模组吞噬动作完成时，该 NPC 会交由 Devourment 模组处理。";

			PSString T1 = "启用或禁用与 Alternate Conversation Camera 模组的兼容。\n"
							 "启用后，本模组在对话期间的镜头偏移会被禁用。";

			if (ImGui::CollapsingHeader("兼容性", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("Devourment 兼容",&Config::General.bDevourmentCompat, T0, !Runtime::IsDevourmentInstalled());
				ImGui::SameLine();
				ImGuiEx::CheckBox("Alt Conversation Cam. 兼容", &Config::General.bConversationCamCompat, T1, !Runtime::IsAltConversationCamInstalled());
				ImGui::Spacing();

			}
		}

		ImUtil_Unique
		{
			PSString T1 = "根据角色体型调整所有动画速度。";
			PSString T2 = "减少部分声音和视觉效果中的血腥程度。";

			if (ImGui::CollapsingHeader("杂项", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("动态动画速度", &Config::General.bDynamicAnimspeed, T1);
				ImGui::SameLine();
				ImGuiEx::CheckBox("减少血腥", &Config::General.bLessGore, T2);
				ImGui::Spacing();
			}
		}

		//------ Experimental

	    ImUtil_Unique 
		{

	        PSString T0 = "男性角色支持：\n"
	                      "此功能不提供支持保障。\n"
	                      "本模组主要按女性 NPC 设计，\n"
	                      "并始终假设玩家/追随者为女性。\n"
	                      "动画观感可能不佳，甚至可能引发问题。\n"
	                      "请自行承担风险。";

			PSString T1 = "对场景中的所有 NPC 应用计算成本较高的伤害计算。\n"
		              "此开关可能极其消耗 FPS，强烈建议保持关闭。";
			
			PSString T2 = "启用或禁用对 ini 中 fActivatePickLength 与 fActivatePickRadius 的动态修改。\n"
		              "它会从默认的 180 和 18 改为 180 和 18 * 玩家体型。";



	        if (ImGui::CollapsingHeader("实验性", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("允许男性角色", &Config::General.bEnableMales, T0);
				ImGui::SameLine();
				ImGuiEx::CheckBox("对所有角色应用体型效果", &Config::General.bAllActorSizeEffects, T1);
				ImGuiEx::CheckBox("覆盖物品/NPC 互动距离", &Config::General.bOverrideInteractionDist, T2);
	        }
	    }
	}

	void CategoryGeneral::DrawRight() {


		//----- Dynamic Scale

	    ImUtil_Unique 
		{

			PSString T0 = "此开关启用自动体型调整：\n"
			              "如果玩家或追随者太大而无法适应房间，会被临时缩小到约为当前房间高度的 90%%。\n"
			              "离开小房间后，会恢复到之前的体型。";

			PSString T1 = "临时缩小目标角色，使其适应正在使用的家具。";

			if (ImGui::CollapsingHeader("动态体型", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::CheckBox("适应房间（玩家）", &Config::General.bDynamicSizePlayer, T0);
				ImGui::SameLine();
				ImGuiEx::CheckBox("适应房间（追随者）", &Config::General.bDynamicSizeFollowers, T0);

				ImGuiEx::CheckBox("适应家具（玩家）", &Config::General.bDynamicFurnSizePlayer, T1);
				ImGui::SameLine();
				ImGuiEx::CheckBox("适应家具（追随者）", &Config::General.bDynamicFurnSizeFollowers, T1);
			}

			ImGui::Spacing();
	    }

		ImUtil_Unique
		{
			PSString T2Help = "移动速度钳制会在 NPC 超过指定体型阈值后逐渐降低移动速度。\n"
							  "这能防止大型 NPC 以不真实的速度移动。\n"
							  "速度降低会在起始阈值与最大阈值之间平滑缩放。";

			PSString T2_1 = "禁用冲刺并开始降低速度的体型。";
			PSString T2_2 = "NPC 速度倍率完全钳制到下方设置值时的体型。";
			PSString T2_3 =
				"控制应用体型速度钳制后，NPC 最多能变得多慢。\n"
				"例如，80%% 表示即使超过最大钳制阈值，NPC 仍至少以正常跑步/慢跑速度的 80%% 移动。\n"
				"0%% 实际上会禁用此功能。";


			if (ImGui::CollapsingHeader("移动", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::HelpText("这是什么", T2Help);
				ImGuiEx::SliderF("NPC 速度钳制起点", &Config::General.fNPCMaxSpeedMultClampStartAt, 1.0f, 20.0f, T2_1, "大于 %.1fx 时");
				ImGuiEx::SliderF("NPC 最大钳制", &Config::General.fNPCMaxSpeedMultClampMaxAt, Config::General.fNPCMaxSpeedMultClampStartAt, Config::General.fNPCMaxSpeedMultClampStartAt + 20.f, T2_2, "%.1fx 时");
				ImGuiEx::SliderF("最低速度偏移 %", &Config::General.fNPCMaxSpeedMultLerpTargetPercent, 0.0f, 100.f, T2_3, "步行速度的 %.0f%%");
				ImGui::Spacing();
			}
		}

		//----- HH

	    ImUtil_Unique 
		{

	        PSString T0 = "为穿高跟鞋的角色启用高度调整/修正。";
	        PSString T1 = "使用家具时禁用高跟鞋高度调整，以允许其他模组处理。";

	        if (ImGui::CollapsingHeader("高跟鞋", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::CheckBox("高度调整", &Config::General.bEnableHighHeels, T0);

				ImGui::SameLine();

		if (ImGuiEx::CheckBox("使用家具时禁用", &Config::General.bHighheelsFurniture, T1, !Config::General.bEnableHighHeels)){
			ConfigModHandler::DoHighHeelStateReset();
	            }

				ImGui::Spacing();

	        }
	    }


		//------------- Looting

	    ImUtil_Unique 
		{

	        PSString T0 = "切换吞噬、缩小致死或碾压等动作是否生成包含死者物品栏的战利品堆。\n"
	                         "如果禁用，死亡时物品栏会自动转移给击杀者。";

	        if (ImGui::CollapsingHeader("战利品", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("玩家：生成战利品堆",&Config::General.bPlayerLootpiles, T0);
				ImGui::SameLine();
				ImGuiEx::CheckBox("追随者：生成战利品堆",&Config::General.bFollowerLootpiles, T0);
	            ImGui::Spacing();

	        }
	    }


		//------------- Gravity

		ImUtil_Unique 
		{

			PSString T0 = "根据体型启用或禁用重力加速度。\n"
							"- 启用后，重力会随玩家成长略微增加：1.0 * sqrt(size)\n"
							"  （这表示大型玩家会掉得更快，但不会过快。）\n"
							"- 禁用后，重力保持 1.0 不变。\n"
							"- 此选项仅作用于玩家。";

			PSString T1 = "部分动画在落地时事件时机较差。\n"
							"- 如果动画时机不佳，伤害区域可能生成在空中而打不到任何人。\n"
							"- 使用此滑条调整延迟，0 = 无延迟，1 = 落地时延迟 1 秒。";

			PSString T2 = "启用“影响玩家重力”时，调整额外跳跃落地延迟。\n"
							"- 这可能是必要的，因为动画可能没有足够时间在地面触发脚部事件。\n"
							"- 否则可能无法对地面敌人造成任何伤害。\n"
							"- 此值会叠加在原始跳跃落地延迟之上。\n\n"
							"- 此值还会进一步乘以重力强度。";

			if (ImGui::CollapsingHeader("跳跃", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("影响玩家重力", &Config::General.bAlterPlayerGravity, T0);
				ImGuiEx::SliderF("伤害效果延迟 - 重力", &Config::General.fAdditionalJumpEffectDelay_Gravity, 0.0f, 1.0f, T2, "%.2fs", !Config::General.bAlterPlayerGravity);
				ImGuiEx::SliderF("伤害效果延迟", &Config::General.fAdditionalJumpEffectDelay, 0.0f, 1.0f, T1, "%.2fs");

				ImGui::Spacing();
			}
		}
	}
}
