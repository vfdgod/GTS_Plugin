#include "UI/Windows/Settings/Categories/Advanced.hpp"

#include "UI/Core/ImUtil.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/Slider.hpp"

#include "Managers/Animation/Utils/CooldownManager.hpp"
#include "Utils/Plugin/Logger.hpp"

namespace GTS {

	CategoryAdvanced::CategoryAdvanced() {
		m_name = "高级";
	}

    bool CategoryAdvanced::IsVisible() const {
        return Config::Hidden.IKnowWhatImDoing;
    }

    void CategoryAdvanced::SetVisible(const bool a_visible) {
        Config::Hidden.IKnowWhatImDoing = a_visible;
    }

    void CategoryAdvanced::DrawLeft() {

        ImUtil_Unique 
		{

            PSString T0 = "显示或隐藏这个页面。";

            if (ImGui::CollapsingHeader("高级设置", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("启用/禁用此页面", &Config::Hidden.IKnowWhatImDoing, T0);

                ImGui::Spacing();
            }
        }

        ImUtil_Unique 
		{

            PSString T0 = "设置日志级别。级别越高，写入 GTSPlugin.log 的信息越多。";
            PSString T1 = "开启后，当前存档会优先读取并写入 Data/SKSE/Plugins/GTSPlugin/SharedSettings.toml，\n"
                          "用于共享除角色/世界进度外的模组配置。\n"
                          "关闭后不会恢复旧的存档配置，只是从下一次保存开始改回按存档保存。";

            if (ImGui::CollapsingHeader("日志", ImUtil::HeaderFlagsDefaultOpen)) {

                if (ImGuiEx::ComboEx<spdlog::level::level_enum>("日志级别", Config::Advanced.sLogLevel, T0, false, true)) {
	                logger::SetLevel(Config::Advanced.sLogLevel.c_str());
				}

                ImGuiEx::CheckBox("全局共享配置", &Config::Advanced.bShareSettingsGlobally, T1);

				ImGui::Spacing();

			}
        }

        ImUtil_Unique 
		{

            PSString T0 = "启用/禁用对玩家体力和魔力的 ActorValue 消耗。";
            PSString T1 = "启用后，尺寸相关技能会保留冷却。关闭后，主要技能冷却会被一并跳过。";
            PSString T2 = "限制滑条的手动输入范围。\n"
                          "开启后，Ctrl+单击滑条输入数值时会被限制在 UI 范围内。\n"
                          "关闭后，可以手动输入超出范围的值；普通拖动滑条本身始终受范围限制。";
            PSString T3 = "开启后，玩家不使用龙吼也会获得 Tiny Calamity 的部分被动特性：\n"
                          "- 冲刺蓄力冲撞/碾碎\n"
                          "- 尺寸动作、脚步、踩踏、投掷、抓握等强化\n"
                          "- Absorb 额外缩小强度\n"
                          "- 生命抗性、攻击和负重按 Tiny Calamity 规则结算\n\n"
                          "不会启动龙吼本体的持续时间、体型压制或 Shrinking Gaze；真正使用龙吼时不会重复叠加。";
            PSString T4 = "开启后，玩家处于常驻 Tiny Calamity 特性时，普通踩踏、践踏和下踩的首次落脚会尽量保留目标 1 点生命，方便接续碾磨或持续踩踏。\n"
                          "碾磨和持续踩踏本身仍可正常杀死目标。";

            if (ImGui::CollapsingHeader("调试/作弊", ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("启用属性消耗", &Config::Advanced.bDamageAV, T0);
                ImGuiEx::CheckBox("启用尺寸技能冷却", &Config::Advanced.bCooldowns, T1);
                ImGuiEx::CheckBox("限制手动输入范围", &Config::Advanced.bEnforceUIClamps, T2);
                ImGuiEx::CheckBox("玩家常驻 Tiny Calamity 特性", &Config::Advanced.bPlayerTinyCalamityBonus, T3);
                ImGuiEx::CheckBox("Tiny Calamity 仁慈模式", &Config::Advanced.bPlayerTinyCalamityMercy, T4, !Config::Advanced.bPlayerTinyCalamityBonus);

                if (ImGuiEx::Button("清空技能冷却")) {
                    CooldownManager::GetSingleton().Reset();
                }

				{   // GTS Skill Level Setter
                    std::string Name = "None";
                    Actor* target = nullptr;
                    if (static const auto pickdata = CrosshairPickData::GetSingleton()) {
                        if (const auto actorhandle = pickdata->targetActor) {
                            if (const auto actorref = actorhandle.get().get()) {
                                if (const auto actor = skyrim_cast<Actor*>(actorref)) {
                                    target = actor;
                                }
                            }
                        }
                    }

                    if (!target) {
                        target = PlayerCharacter::GetSingleton();
                    }

                    if (auto data = Persistent::GetActorData(target)) {
                        float& level = (target == PlayerCharacter::GetSingleton()) ? (Runtime::GetGlobal(Runtime::GLOB.GTSSkillLevel)->value) : data->fGTSSkillLevel;
                        ImGui::Text("当前目标：%s [%0.f]", target->GetName(), level);

                        static int TargetLevel = 50;
                        ImGui::InputInt("目标等级", &TargetLevel, 1, 10);
                        if (ImGuiEx::Button("设置 GTS 等级")) {
                            level = static_cast<float>(TargetLevel);
                        }
                    }
                }
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
            PSString T8 = "只有玩家与目标的体型差达到该倍率时，目标才会被视为适合踩踏辅助。";

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
    }

    void CategoryAdvanced::DrawRight() {

        ImUtil_Unique 
		{

	        PSString T1 = "把玩家视为 NPC，使玩家也会执行随机动画。";
	        PSString T2 = "启用对 Devourment 的实验性 AI 支持，用于部分替代 DV 自带的伪 AI。";
	        PSString T3 = "设置触发 DV 动作的概率。";

	        if (ImGui::CollapsingHeader("实验功能",ImUtil::HeaderFlagsDefaultOpen)) {
                ImGuiEx::CheckBox("玩家 AI", &Config::Advanced.bPlayerAI, T1);

                ImGuiEx::CheckBox("Devourment AI", &Config::Advanced.bEnableExperimentalDevourmentAI, T2, !Runtime::IsDevourmentInstalled());
                ImGuiEx::SliderF("Devourment AI 概率", &Config::Advanced.fExperimentalDevourmentAIProb, 1.0f, 100.0f, T3,"%.0f%%", !Config::Advanced.bEnableExperimentalDevourmentAI, !Runtime::IsDevourmentInstalled());

	            ImGui::Spacing();
	        }
        }

        ImUtil_Unique
		{

            PSString T0 = "对最终的 GetAnimationSlowdown 结果再乘上一个系数。";
            PSString T1 = "修改计算动画减速量的 \"SoftCore\" 公式参数。";
            PSString T2 = "当 IsGTSBusy() == true 时，是否强制把动画速度设为 1x。";
            PSString T3 = "定义动画速度公式允许的最低值（默认 0.1f）。";

            if (ImGui::CollapsingHeader("动画速度", ImUtil::HeaderFlagsDefaultOpen)) {

                ImGuiEx::CheckBox("GTS 动作始终 1x 速度", &Config::Advanced.bGTSAnimsFullSpeed, T2);
                ImGuiEx::SliderF("玩家动画速度", &Config::Advanced.fAnimSpeedAdjMultPlayer, 0.1f, 3.0f, T0);
                ImGuiEx::SliderF("追随者动画速度", &Config::Advanced.fAnimSpeedAdjMultTeammate, 0.1f, 3.0f, T0);
                ImGuiEx::SliderF("允许的最低动画速度", &Config::Advanced.fAnimspeedLowestBoundAllowed, 0.01f, 1.0f, T3);

                const float PlayerSlowDown = GetAnimationSlowdown(PlayerCharacter::GetSingleton());

                ImGui::Spacing();

                ImGui::Text("玩家减速值：%.2fx", PlayerSlowDown);

                ImGui::Spacing();

                //https://www.desmos.com/calculator/vyofjrqmrn
                ImGui::Text("动画公式");
                ImGuiEx::SliderF3("参数 K, N, S", &Config::Advanced.fAnimSpeedSoftCore.at(0), 0.0f, 10.0f, T1, "%.3f");
                ImGuiEx::SliderF2("参数 O, A", &Config::Advanced.fAnimSpeedSoftCore.at(3), 0.0f, 10.0f, T1, "%.3f");
                if (ImGuiEx::Button("重置")) {
                    Config::Advanced.fAnimSpeedSoftCore = { 0.140f, 0.540f, 1.350f, 1.0f, 0.0f };
                }

                ImGui::Spacing();
            }
        }

        ImUtil_Unique 
		{

            PSString THelp = "这里可以清除这个模组的内部角色数据。\n"
                             "请尽量在类似 qasmoke 这样的测试单元执行。只会删除未加载角色的数据。\n"
                             "执行后你必须保存、退出并重启游戏，否则很容易把存档状态搞乱。";

	        if (ImGui::CollapsingHeader("数据管理",ImUtil::HeaderFlagsDefaultOpen)) {

                if (ImGuiEx::Button("清空 Persistent", "清除 persistent 中的全部数据", false, 1.0f)) {
					if (auto tes = TES::GetSingleton()) {
						tes->PurgeBufferedCells();
					}
                    logger::critical("Purged cell buffers in preperation of persistent erase.");
                	Persistent::EraseUnloadedData();
                }

                ImGui::SameLine();

                if (ImGuiEx::Button("清空 Transient", "清除 transient 中的全部数据", false, 1.0f)) {
					if (auto tes = TES::GetSingleton()) {
						tes->PurgeBufferedCells();
					}
                    logger::critical("Purged cell buffers in preperation of transient erase.");
                    Transient::EraseUnloadedData();
                }

                ImGui::SameLine();

                ImGui::TextColored(ImUtil::Colors::Warning, "说明 (!)");
                ImGuiEx::Tooltip(THelp, true);

                ImGui::Spacing();

	        }
        }
    }
}
