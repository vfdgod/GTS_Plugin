#include "UI/Windows/Settings/Categories/Camera.hpp"

#include "UI/Controls/Slider.hpp"
#include "UI/Core/ImUtil.hpp"

#include "Config/Config.hpp"

#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/Text.hpp"

namespace GTS {

	static void DrawCameraOffsets(const char* a_title, const char* a_toolip, std::array<float, 3>* a_offsets) {

	    ImGui::BeginGroup();

		ImGuiEx::SliderF3(a_title, &a_offsets->at(0), -175.f, 175.f, nullptr, "%.1f");

	    ImGui::EndGroup();

	    // Applies to the whole group
	    if (ImGui::IsItemHovered()) {
	        ImGui::SetTooltip(a_toolip);
	    }
	}

	static void DrawCameraSettings(CameraOffsets_t* a_set, const char* a_title) {

	    PSString T0 = "选择镜头要追踪的双足骨架骨骼。";

	    if (ImGui::CollapsingHeader(a_title, ImUtil::HeaderFlagsDefaultOpen)) {
			ImGuiEx::ComboEx<LCameraTrackBone_t>("居中骨骼", a_set->sCenterOnBone, T0);

	        ImUtil_Unique 
		{
	            DrawCameraOffsets(
	                "偏移 | 站立",
	                "调整站立时的镜头偏移。\n"
					"左/右 | 前/后 | 上/下",
	                &a_set->f3NormalStand
	            );
	        }

	        ImUtil_Unique 
		{
	            DrawCameraOffsets(
	                "偏移 | 站立战斗",
	                "调整站立且处于战斗中时的镜头偏移。\n"
					"左/右 | 前/后 | 上/下",
	                &a_set->f3CombatStand
	            );
	        }

	        ImUtil_Unique 
		{
	            DrawCameraOffsets(
	                "偏移 | 爬行",
	                "调整潜行、爬行或趴伏时的镜头偏移。\n"
					"左/右 | 前/后 | 上/下",
	                &a_set->f3NormalCrawl
	            );
	        }

	        ImUtil_Unique 
		{
	            DrawCameraOffsets(
	                "偏移 | 爬行战斗",
	                "调整潜行、爬行或趴伏且处于战斗中时的镜头偏移。\n"
					"左/右 | 前/后 | 上/下",
	                &a_set->f3CombatCrawl
	            );
	        }

	        ImGui::Spacing();
	    }
	}

	CategoryCamera::CategoryCamera() {
		m_name = "镜头";
	}

	void CategoryCamera::DrawLeft() {

	    ImUtil_Unique 
		{

	        PSString T0 = "调整玩家执行动作时的镜头震动强度。";
			PSString T1 = "调整由自身体型或体型动作造成的第一人称镜头震动强度。";
			PSString T2 = "调整 NPC 造成的镜头震动强度。";

	        if (ImGui::CollapsingHeader("镜头震动", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::SliderF("玩家总震动强度", &Config::Camera.fCameraShakePlayer, 0.1f, 3.0f, T0, "%.2fx");
				ImGuiEx::SliderF("玩家第一人称震动强度", &Config::Camera.fCameraShakePlayerFP, 0.0f, 1.0f, T1, "%.2fx");
				ImGuiEx::SliderF("NPC 总震动强度", &Config::Camera.fCameraShakeOther, 0.1f, 3.0f, T2, "%.2fx");

	            ImGui::Spacing();
	        }
	    }

	    ImUtil_Unique 
		{

	        PSString T0 = "调整爬行时的镜头高度倍率。\n"
							 "第一人称 | 第三人称\n\n"
							 "注意：如果使用 SmoothCam，第三人称可能无法正常工作。";

			if (ImGui::CollapsingHeader("爬行高度", ImUtil::HeaderFlagsDefaultOpen)) {
				//Temp Store
				static constexpr std::array Temp = { &Config::Camera.fFPCrawlHeightMult,  &Config::Camera.fTPCrawlHeightMult };

				ImGuiEx::SliderF2("爬行高度倍率", Temp.at(0), 0.01f, 1.0f, T0, "%.1fx");
	            ImGui::Spacing();
	        }
	    }

	    ImUtil_Unique 
		{

	        PSString T0 = "启用镜头与角色的碰撞。";
	        PSString T1 = "启用镜头与树木的碰撞。";
	        PSString T2 = "启用镜头与杂物（带物理的物体）的碰撞。";
	        PSString T3 = "启用镜头与地形的碰撞。";
	        PSString T4 = "启用镜头与静态物体的碰撞（基本指任何实体、不可移动物体）。";
	        PSString T5 = "设置上述碰撞选项开始生效的体型。";

	        if (ImGui::CollapsingHeader("镜头碰撞", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("与角色碰撞", &Config::Camera.bCamCollideActor, T0);
	            ImGui::SameLine();
				ImGuiEx::CheckBox("与树木碰撞", &Config::Camera.bCamCollideTree, T1);
				ImGuiEx::CheckBox("与杂物碰撞", &Config::Camera.bCamCollideDebris, T2);
	            ImGui::SameLine();
				ImGuiEx::CheckBox("与地形碰撞", &Config::Camera.bCamCollideTerrain, T3);
				ImGuiEx::CheckBox("与静态物体碰撞", &Config::Camera.bCamCollideStatics, T4);
				ImGuiEx::SliderF("生效体型", &Config::Camera.fModifyCamCollideAt, 0.0f, 50.0f, T5, "%.1fx");
	            ImGui::Spacing();
	        }
	    }

	    ImUtil_Unique 
		{

	        PSString T0 = "偏移第三人称镜头的最小缩放距离。\n"
		              "结合最大距离一起，影响镜头视角切换到第一人称时与玩家的距离。";

	        PSString T1 = "偏移第三人称镜头的最大缩放距离。\n"
		              "数值越高，镜头会拉得越远。\n"
		              "结合最小距离一起，影响镜头视角切换到第一人称时与玩家的距离。";

			PSString T2 = "调整镜头缩放步进之间的过渡速度。";

	        PSString T3 = "调整镜头缩放步进除数。\n"
						  "数值越低，缩放步数越多；\n"
					  "数值越高，缩放步数越少。\n";

			PSString T4 = "切换本模组是否覆盖 Skyrim 的镜头设置。\n"
						  "注意：禁用后需要重启游戏，原始值才会重新应用。\n\n"
						  "建议保持启用。";

	        PSString THelp = "这些设置与 skyrim.ini 中的设置相同。\n"
				             "添加到这里是为了方便调整。\n\n"
				             "注意 1：这里的设置会持续覆盖游戏设置，\n"
				             "无论你在任何 ini 文件中设置了什么值，或其他模组修改了它们，都会被这里的值覆盖。\n\n"
							 "注意 2：强烈建议不要修改距离设置。本模组的镜头系统在这些值保持默认时效果最好。\n\n"
						 "默认值：\n"
						 " - 最小距离：150.0\n"
							 " - 最大距离：600.0\n"
							 " - 缩放速度：0.8\n"
							 " - 缩放步进：0.075\n";

	        if (ImGui::CollapsingHeader("Skyrim 镜头设置", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::HelpText("这是什么", THelp);

				ImGuiEx::CheckBox("启用调整", &Config::Camera.bEnableSkyrimCameraAdjustments, T4);

				ImGui::BeginDisabled(!Config::Camera.bEnableSkyrimCameraAdjustments);

				ImGuiEx::SliderF("最小距离", &Config::Camera.fCameraDistMin, -100.0f, 300.0f, T0, "%.1f");
				ImGuiEx::SliderF("最大距离", &Config::Camera.fCameraDistMax, 50.0f, 1200.0f, T1, "%.1f");
				ImGuiEx::SliderF("缩放速度", &Config::Camera.fCameraZoomSpeed, 0.1f, 10.0f, T2, "%.1f");
				ImGuiEx::SliderF("缩放步进", &Config::Camera.fCameraIncrement, 0.01f, 0.25f, T3, "%.3f");

				ImGui::EndDisabled();


	            ImGui::Spacing();
	        }
	    }

	}

	void CategoryCamera::DrawRight() {

	    ImUtil_Unique 
		{

	        PSString T0 = "启用自动镜头。";
			PSString T1 = "修改第三人称镜头模式。";

			//Hack
            auto CamState = std::bit_cast<int*>(&Persistent::TrackedCameraState.value);

	        if (ImGui::CollapsingHeader("自动镜头", ImUtil::HeaderFlagsDefaultOpen)) {
				ImGuiEx::CheckBox("启用自动镜头", &Config::Camera.bAutomaticCamera, T0);
				ImGuiEx::IComboEx<LCameraMode_t>("镜头模式", CamState, T1, !Config::Camera.bAutomaticCamera);
		ImGui::Spacing();
	        }
	    }

	    ImGui::BeginDisabled(!Config::Camera.bAutomaticCamera);

	    ImUtil_Unique 
		{
	        DrawCameraSettings(&Config::Camera.OffsetsNormal, "普通镜头");
	    }

	    ImUtil_Unique 
		{
	        DrawCameraSettings(&Config::Camera.OffsetsAlt, "替代镜头");
	    }

		ImGui::EndDisabled();

		ImUtil_Unique
		{
			if (ImGui::CollapsingHeader("镜头视锥", ImUtil::HeaderFlagsDefaultOpen)) {

				PSString T1 = "动态修改镜头近裁剪视锥距离，以修复体型很小时的穿模问题。\n"
					          "可能引入阴影移动/消失等视觉问题。\n\n"
					          "小于 1.0x 体型时开始生效。\n"
					          "超过 1.0x 体型后会自动禁用。\n\n"
					          "注意：可能与其他也修改此值的模组冲突。";

				PSString T2 = "动态修改镜头远裁剪视锥距离，以修复体型极大时角色/地形/LOD 消失的问题。\n"
					          "注意：可能与其他也修改此值的模组冲突。";

				ImGuiEx::CheckBox("动态视锥 - 近裁剪", &Config::Camera.bEnableAutoFNearDist, T1);
				ImGuiEx::CheckBox("动态视锥 - 远裁剪", &Config::Camera.bEnableAutoFFarDist, T2);
			}
		}
	}
}
