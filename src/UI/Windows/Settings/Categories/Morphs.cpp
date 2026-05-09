#include "UI/Windows/Settings/Categories/Morphs.hpp"
#include "API/Racemenu.hpp"
#include "Config/Config.hpp"

#include "UI/GTSMenu.hpp"
#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/CollapsingTabHeader.hpp"
#include "UI/Controls/Slider.hpp"
#include "UI/Controls/Text.hpp"

#include "UI/Core/ImUtil.hpp"
#include "UI/Windows/Settings/SettingsWindow.hpp"

namespace {

    PSString THelp = "- 修改 Morph 可能非常消耗 FPS！\n"
                     "这里可以配置在执行特定动作时应用到角色身上的 morph 目标。\n\n"
                     "Morph 会按类别划分（如胸部、腹部等）。\n"
                     "每个类别可定义多个 Racemenu/Bodyslide morph（最多 16 个），并组合到一起。\n"
                     "可在 Outfit Studio 中加载你使用的身体模组，查看 morph 名称列表。";
                     

	void DrawMorphList(GameplayMorphSettings_t& settings) {

		constexpr int32_t kMax = 16;
		constexpr int32_t kLast = kMax - 1;

		ImGuiEx::HelpText("这是什么", THelp);

		auto mark_modified = [] {
			if (auto* win = dynamic_cast<GTS::SettingsWindow*>(GTS::GTSMenu::WindowManager->wSettings)) {
				win->m_MorphDataWasModified = true;
			}
		};

		if (ImGuiEx::SliderF(
			"倍率",
			&settings.fMultiplier,
			0.1f,
			5.0f,
			"用此数值乘以最终组合后的 morph 形状。",
			"%.2fx")) {
			mark_modified();
		}

		ImGui::SeparatorText("Morph 列表");

		static int32_t activeEditIndex = -1;

		auto shift_left_from = [&](int32_t idx) {
			for (int32_t j = idx; j < kLast; ++j) {
				settings.MorphNames[j] = std::move(settings.MorphNames[j + 1]);
				settings.MorphScales[j] = settings.MorphScales[j + 1];
			}
			settings.MorphNames[kLast].clear();
			settings.MorphScales[kLast] = 0.0f;
		};

		auto last_used_index = [&]() -> int32_t {
			for (int32_t i = kLast; i >= 0; --i) {
				if (!settings.MorphNames[i].empty()) {
					return i;
				}
			}
			return -1;
		};

		// Always show one empty row after the last non-empty entry (if capacity allows).
		const int32_t lastUsed = last_used_index();
		const int32_t renderUpTo = std::min(lastUsed + 1, kLast);

		const float itemWidth = ImGui::GetWindowWidth() * GTS::Config::UI.fItemWidth * 0.5f;

		for (int32_t i = 0; i <= renderUpTo; ++i) {
			ImGui::PushID(i);

			bool changed = false;

			ImGui::PushItemWidth(itemWidth);

			const bool nameChanged = ImGui::InputText(
				"##name",
				&settings.MorphNames[i],
				ImGuiInputTextFlags_AutoSelectAll
			);

			const bool isActive = ImGui::IsItemActive();

			if (isActive) {
				activeEditIndex = i;
			}
			else if (activeEditIndex == i) {
				// Field just lost focus: if it is empty and not the last slot, remove it by shifting.
				if (settings.MorphNames[i].empty() && i < kLast) {
					shift_left_from(i);
					changed = true;
				}
				activeEditIndex = -1;
			}

			ImGui::SameLine();

			const bool scaleChanged = ImGui::InputFloat(
				"##Scale",
				&settings.MorphScales[i],
				0.01f,
				0.1f,
				"%.2fx",
				ImGuiInputTextFlags_AutoSelectAll
			);

			ImGui::PopItemWidth();

			changed = changed || nameChanged || scaleChanged;

			// Delete button only for non-empty rows
			if (!settings.MorphNames[i].empty()) {
				ImGui::SameLine();
				if (ImGuiEx::ImageButton("##delete", ImageList::Export_Delete, 18, "删除此 morph 条目")) {
					shift_left_from(i);
					changed = true;

					// If we deleted the row being edited, clear edit tracking.
					if (activeEditIndex == i) {
						activeEditIndex = -1;
					}
				}
			}

			if (changed) {
				mark_modified();
			}

			ImGui::PopID();
		}
	}

}

namespace GTS {

    CategoryMorphs::CategoryMorphs() {
        m_name = "变形";
    }

    void CategoryMorphs::DrawLeft() {

        // ------- Morph Settings

        ImUtil_Unique
        {

			static ImGuiEx::CollapsingTabHeader MorphSettings(
	            "Morph 设置",
	            std::initializer_list<std::string>
	            {
	                "胸部",
		"腹部"
	            }
            );

            if (!Racemenu::Loaded()) {
                MorphSettings.SetDisabledState(true);
                MorphSettings.SetExtraInfo("无法获取 Racemenu API");
            }

            if (ImGuiEx::BeginCollapsingTabHeader(MorphSettings)) {
                // Content based on active tab
                switch (MorphSettings.GetActiveTab()) {
                    case 0: {
                        ImGui::BeginDisabled(!Config::Gameplay.ActionSettings.bEnlargeBreastsOnAbsorption);
                        DrawMorphList(Config::Gameplay.ActionSettings.MorphListBreasts);
                        ImGui::EndDisabled();
                    } break;

					case 1: {
						ImGui::BeginDisabled(!Config::Gameplay.ActionSettings.bEnableBellyMorph);
						DrawMorphList(Config::Gameplay.ActionSettings.MorphListBelly);
						ImGui::EndDisabled();
					} break;

                    default: break;
                }

            }
	ImGuiEx::EndCollapsingTabHeader(MorphSettings);
        }
    }

    void CategoryMorphs::DrawRight() {

        ImGui::BeginDisabled(!Racemenu::Loaded());

        ImUtil_Unique
        {
            if (ImGui::CollapsingHeader("性能", ImUtil::HeaderFlagsDefaultOpen)) {

                PSString T0 = "应用更新前的最小 morph 变化阈值。\n"
							  "较高值会跳过较小变化以降低性能开销，但可能让过渡不够平滑。\n"
							  "设为 0 可获得最大平滑度，但性能开销更高。";

                ImGuiEx::SliderF("更新阈值", &Config::Gameplay.ActionSettings.fMorphEPS, 0.0f, 0.3f, T0, "%.3fx");
                ImGui::Spacing();
            }
        }

        ImUtil_Unique
        {

            if (ImGui::CollapsingHeader("胸部 Morph 设置", ImUtil::HeaderFlagsDefaultOpen)) {

                PSString T0 = "完成特定动作（如乳沟吸收）后启用胸部增长。\n"
                                "- 修改 Morph 可能非常消耗 FPS！";
                PSString T1 = "切换 morph 是否应随时间降低。\n"
                                "- 修改 Morph 可能非常消耗 FPS！";
				PSString T2 = "设置胸部随时间缩小的速率。";
				PSString T3 = "设置每次吸收后胸部增加的量。";

                ImGuiEx::CheckBox("吸收后增大胸部", &Config::Gameplay.ActionSettings.bEnlargeBreastsOnAbsorption, T0);

				ImGui::BeginDisabled(!Config::Gameplay.ActionSettings.bEnlargeBreastsOnAbsorption);
                {
                    ImGuiEx::CheckBox("随时间缩小", &Config::Gameplay.ActionSettings.bShrinkBreastsOverTime, T1);
                    ImGuiEx::SliderF("缩小速率", &Config::Gameplay.ActionSettings.fBreastShrinkRate, 0.01f, 0.5f, T2, "%.2fx", !Config::Gameplay.ActionSettings.bShrinkBreastsOverTime);
                    ImGuiEx::SliderF("每次吸收增加量", &Config::Gameplay.ActionSettings.fBreastsAbsorbIncrementBy, 0.1f, 5.0f, T3, "%.2fx");
                }
				ImGui::EndDisabled();

                ImGui::Spacing();
            }
        }

        ImUtil_Unique
        {

            if (ImGui::CollapsingHeader("腹部 Morph 设置", ImUtil::HeaderFlagsDefaultOpen)) {

                PSString T0 = "吞噬动作时启用腹部增长。\n"
                            "- 修改 Morph 可能非常消耗 FPS！";

				PSString T1 = "切换 morph 是否应随时间降低。\n"
					"- 修改 Morph 可能非常消耗 FPS！";

				PSString T2 = "被吃掉的 NPC 被吸收后，腹部通常会缩小。\n"
				  "你也可以启用额外的渐进缩小，并在此设置缩小速率。";

	PSString T3 = "设置每吃掉一个 NPC 后增加的量。";

                ImGuiEx::CheckBox("吞噬时增大腹部", &Config::Gameplay.ActionSettings.bEnableBellyMorph, T0);

				ImGui::BeginDisabled(!Config::Gameplay.ActionSettings.bEnableBellyMorph);
				{
					ImGuiEx::CheckBox("随时间缩小", &Config::Gameplay.ActionSettings.bShrinkBellyOverTime, T1);

					ImGuiEx::SliderF("缩小速率", &Config::Gameplay.ActionSettings.fBellyShrinkRate, 0.001f, 0.2f, T2, "%.3fx", !Config::Gameplay.ActionSettings.bShrinkBellyOverTime);
					ImGuiEx::SliderF("每次吞噬增加量", &Config::Gameplay.ActionSettings.fBellyAbsorbIncrementBy, 0.1f, 1.0f, T3, "%.2fx");
				}
				ImGui::EndDisabled();

                ImGui::Spacing();
            }
        }

		ImGui::EndDisabled();
    }
}
