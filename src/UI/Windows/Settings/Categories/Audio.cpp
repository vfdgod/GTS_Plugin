#include "UI/Windows/Settings/Categories/Audio.hpp"

#include "Config/Config.hpp"

#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/Text.hpp"
#include "UI/Controls/ToolTip.hpp"
#include "UI/Core/ImUtil.hpp"

using namespace GTS;

namespace {

	void SelectVoiceBank(RE::Actor* a_actor) {

		if (!a_actor) return;

		static constexpr std::array<const char*, 9> EntriesFemale = {
			"GTS 语音",
			"SL 女性语音 1",
			"SL 女性语音 2",
			"SL 女性语音 3",
			"SL 女性语音 4",
			"SL 女性语音 5",
			"SL 女性语音 6",
			"SL 女性语音 7",
			"SL 女性语音 8"
		};

		if (auto ActorData = Persistent::GetActorData(a_actor)) {
			ImGui::PushID(a_actor);
			int CurrentIndex = ActorData->iVoiceBankIndex;
			if (ImGui::BeginCombo(a_actor->GetName(), EntriesFemale[CurrentIndex])) {
				for (int i = 0; i < EntriesFemale.size(); ++i) {
					const bool IsSelected = (CurrentIndex == i);
					if (ImGui::Selectable(EntriesFemale[i], IsSelected)) {
						ActorData->iVoiceBankIndex = static_cast<uint8_t>(i);
					}
					if (IsSelected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopID();
		}
	}
}

namespace GTS {

	CategoryAudio::CategoryAudio() {
		m_name = "音频";
	}

	void CategoryAudio::DrawLeft(){

	    ImUtil_Unique 
		{

	        PSString T0 = "当玩家体型达到或超过指定阈值时启用脚步声。";
	        PSString T1 = "在缓慢成长等法术期间启用呻吟声音。";

			PSString T2 = "为呻吟/笑声启用类似脚步声的体型变体，但不会进行混合。\n"
		              "如果禁用，将始终使用普通体型声音。\n"
						  "启用后，会根据当前体型范围选择对应声音分类：\n"
						  "normal/x2/x4/x8/x12/x24/x48/x96\n\n"
						  "- 如果没有自定义呻吟/笑声，此选项不会产生效果。\n"
						  "- 声音位置：Data\\Sound\\fx\\GTS\\Moans_Laughs。文件夹默认是空的。\n"
						  "- 可将带混响/回声等效果的声音变体放入对应体型文件夹。";

			PSString T3 = "启用后，只有玩家角色会播放呻吟/笑声。";

			PSString T4 = "体型超过 x2 后，用不同声音集替换自定义高跟鞋体型声音。\n"
		              "未穿高跟鞋时会播放旧声音。\n"
		              "需要 High Heels 的“启用高度调整”开启后才会生效。";

			PSString T5 = "启用时：\n"
						  "- 脚步声音频会尝试平滑切换。\n"
			              "- 可能导致脚步声比未混合时更小。\n"
						  "- 因为混合机制，甚至可能同时播放 2 个声音。\n"
						  "禁用时：\n"
						  "- 一旦超过体型阈值，脚步声会立即切换。";

			PSString T6 = "启用/禁用笑声。";
			PSString T7 = "启用/禁用呻吟声。";

			PSString THelp = "注意：如果安装时选择包含呻吟/笑声，本模组会自带这些声音。\n"
							 "也可以把自己的 .wav 文件放入以下文件夹来添加声音：\n"
						 "（你的 Skyrim 文件夹）\\Data\\Sound\\fx\\GTS\\Moans_Laughs";
	        
	        if(ImGui::CollapsingHeader("声音",ImUtil::HeaderFlagsDefaultOpen)){
				ImGuiEx::HelpText("声音说明", THelp);

				ImGuiEx::CheckBox("启用呻吟", &Config::Audio.bMoanEnable, T6);
				ImGui::SameLine();
				ImGuiEx::CheckBox("启用笑声", &Config::Audio.bLaughEnable, T7);
				ImGuiEx::CheckBox("脚步声",&Config::Audio.bFootstepSounds,T0);
				ImGuiEx::CheckBox("缓慢成长时呻吟",&Config::Audio.bSlowGrowMoans, T1, !Config::Audio.bMoanEnable);

				ImGui::Spacing();

				ImGui::Text("呻吟与笑声");

				ImGui::BeginDisabled(!Config::Audio.bMoanEnable && !Config::Audio.bLaughEnable);
		        {
			        ImGuiEx::CheckBox("体型变体", &Config::Audio.bMoanLaughSizeVariants, T2);
					ImGui::SameLine();
					ImGuiEx::CheckBox("仅玩家", &Config::Audio.bMoanLaughPCExclusive, T3);
		        }
				ImGui::EndDisabled();

				ImGui::Spacing();

				ImGuiEx::CheckBox("替代高跟鞋体型声音", &Config::Audio.bUseOtherHighHeelSet, T4);
				ImGuiEx::CheckBox("平滑混合脚步声", &Config::Audio.bBlendBetweenFootsteps, T5);

	            ImGui::Spacing();
	        }
	    }

		if (Runtime::IsSexlabInstalled()) {

			ImUtil_Unique 
			{

				PSString THelp = "已安装 SexLab。\n"
								 "本模组现在可以把它的语音文件作为替代语音使用。\n\n"
								 "注意：为避免菜单过于杂乱，这里只会列出玩家/当前追随者。\n"
								 "如果此菜单为空，表示当前加载的 NPC 都不符合此功能条件。";
				ImGui::BeginDisabled(!Config::Audio.bMoanEnable);
				if (ImGui::CollapsingHeader("替代语音选项", ImUtil::HeaderFlagsDefaultOpen)) {
					ImGuiEx::HelpText("这是什么", THelp);

					static const auto Player = PlayerCharacter::GetSingleton();

					if (IsFemale(Player)){
						SelectVoiceBank(Player);
					}

					const auto& ActiveTeammates = FindFemaleTeammates();

					if (!ActiveTeammates.empty()) {
						for (const auto Teammate : ActiveTeammates) {
							SelectVoiceBank(Teammate);
						}
					}

					ImGui::Spacing();
				}
				ImGui::EndDisabled();
			}
		}
	}

	void CategoryAudio::DrawRight() {

		ImUtil_Unique 
		{

			PSString TpLow = "体型超过 1.0x 时的最低语音音调倍率；数值越低，角色越大时音调越低。";
			PSString TsMax = "修改音调达到最低值时的目标体型。";
			PSString TpHi = "体型低于 1.0x 时的最高语音音调倍率；数值越低，角色越小时音调越低。";
			PSString TsMin = "修改音调达到最高值时的目标体型。";

			PSString T0 = "切换 NPC 语音是否根据体型修改音调。";
			PSString T2 = "启用/禁用 NPC 被缩小到消失时的死亡声音/尖叫。";
			PSString T3 = "启用/禁用 NPC 被胸部吸收时的死亡声音/尖叫。";
			PSString T4 = "启用/禁用 NPC 被愤怒灾厄杀死时的死亡声音/尖叫。";
			PSString T5 = "启用/禁用 NPC 被拥抱碾压杀死时的死亡声音/尖叫。";
			PSString T6 = "启用/禁用 NPC 被碾压时的死亡声音/尖叫。";
			PSString T7 = "启用/禁用 NPC 被吞噬吃掉时的死亡声音/尖叫。";
			PSString T8 = "呻吟与笑声的衰减范围倍率。数值越大，越远也能听到。";
			PSString T9 = "呻吟与笑声音量倍率。";
			PSString T10 = "切换 GTS 呻吟/笑声是否根据体型修改音调。";
			PSString T11 = "切换血腥音效是否根据 1.0 以下的小体型修改音调。";
			
			if (ImGui::CollapsingHeader("语音",ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("普通语音音调覆盖",&Config::Audio.bEnableVoicePitchOverrideN, T0);
				ImGuiEx::CheckBox("GTS 语音音调覆盖", &Config::Audio.bEnableVoicePitchOverrideG, T10);
				ImGuiEx::CheckBox("小体型血腥音效音调覆盖", &Config::Audio.bEnableGorePitchOverride, T11);

				const bool Enable = !Config::Audio.bEnableVoicePitchOverrideN && !Config::Audio.bEnableVoicePitchOverrideG;

				ImGuiEx::SliderF("最低音调", &Config::Audio.fMinVoiceFreq, 0.65f, 1.0f, TpLow, "%.3fx", Enable);
				ImGuiEx::SliderF("最低音调对应体型", &Config::Audio.fTargetPitchAtScaleMax, 1.5f, 50.0f, TsMax, "%.1fx 时", Enable);
				ImGui::Spacing();
				ImGuiEx::SliderF("最高音调", &Config::Audio.fMaxVoiceFreq, 1.0f, 1.75f, TpHi, "%.3fx", Enable);
				ImGuiEx::SliderF("最高音调对应体型", &Config::Audio.fTargetPitchAtScaleMin, 0.10f, 0.6f, TsMin, "%.2fx 时", Enable);

				ImGui::Spacing();
				
				ImGui::Text("静音死亡声音");
				ImGuiEx::CheckBox("缩小到消失", &Config::Audio.bMuteShrinkToNothingDeathScreams,T2);
				ImGui::SameLine();
				//Store x-Pos of 2nd element
				auto FirstPos = ImGui::GetCursorPosX();

				ImGuiEx::CheckBox("胸部吸收", &Config::Audio.bMuteBreastAbsorptionDeathScreams,T3);

				ImGuiEx::CheckBox("愤怒灾厄", &Config::Audio.bMuteFingerSnapDeathScreams,T4);
				ImGui::SameLine(FirstPos);
				ImGuiEx::CheckBox("拥抱碾压", &Config::Audio.bMuteHugCrushDeathScreams,T5);

				ImGuiEx::CheckBox("碾压", &Config::Audio.bMuteCrushDeathScreams, T6);
				ImGui::SameLine(FirstPos);
				ImGuiEx::CheckBox("吞噬", &Config::Audio.bMuteVoreDeathScreams,T7);
				
				ImGui::Spacing();
				ImGuiEx::SliderF("呻吟/笑声衰减", &Config::Audio.fFallOffMultiplier, 0.02f, 6.0f, T8, "%.2fx");
				ImGuiEx::SliderF("呻吟/笑声音量", &Config::Audio.fVoiceVolumeMult, 0.02f, 1.0f, T9, "%.2fx");

				ImGui::Spacing();
			}
		}
	}
}
