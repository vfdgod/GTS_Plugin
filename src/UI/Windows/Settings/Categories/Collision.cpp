#include "UI/Windows/Settings/Categories/Collision.hpp"

#include "UI/Controls/Slider.hpp"
#include "UI/Core/ImUtil.hpp"
#include "Config/Config.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/Text.hpp"

namespace GTS {


	CategoryCollision::CategoryCollision() {
		m_name = "碰撞";
	}

	void CategoryCollision::DrawLeft() {

		ImUtil_Unique
		{

			PSString T0 = "用于调试目的，可视化碰撞形状。\n"
						  "- 黄色：胶囊碰撞体\n"
						  "- 洋红色：简单凸顶点碰撞体。\n"
						  "- 青色：骨骼驱动凸顶点碰撞体。";

			PSString T1 = "显示 bumper 碰撞形状。\n"
				          "这些仅用于可视化。角色 bumper 碰撞已禁用。";

			if (ImGui::CollapsingHeader("碰撞体调试", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::CheckBox("绘制调试形状", &Config::Collision.bDrawDebugShapes, T0);
				ImGuiEx::CheckBox("绘制 Bumper 形状", &Config::Collision.bDrawBumpers, T1);
				ImGui::Spacing();
			}

		}

		ImUtil_Unique
		{

			PSString T0 = "为追随者启用骨骼驱动碰撞更新。\n"
						  "注意：不建议启用此项。（原因见帮助提示。）";

		    PSString THelp = "骨骼驱动碰撞会追踪特定骨骼（如头部、手臂、腿部）来动态调整碰撞形状。\n"
						     "这会让碰撞更准确，但性能开销更高，因此默认只对玩家启用。\n\n"
						     "简单碰撞体使用统一缩放：基础游戏的原始碰撞形状会整体放大或缩小。\n"
						     "它不会改变形状或跟随单个骨骼，只会在角色体型或状态变化时更新\n"
						     "（例如在行走、潜行或游泳之间切换时）。";

			PSString TDMax = "骨骼驱动碰撞形状允许的最大体型。\n\n"
							 "注意：出于性能原因，这里存在上限。\n"
							 "建议保持在 50.0x。";

			PSString TWidth = "调整碰撞形状相对基础尺寸的水平宽度。";

			if (ImGui::CollapsingHeader("骨骼驱动碰撞体", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::HelpText("这是什么", THelp);

				ImGuiEx::CheckBox("为追随者启用骨骼驱动碰撞更新", &Config::Collision.bEnableBoneDrivenCollisionUpdatesFollowers, T0);
				ImGuiEx::SliderF("最大体型", &Config::Collision.fDynamicColliderMaxUpdateScale, 10.0f, 250.0f, TDMax, "%.2fx");
				ImGuiEx::SliderF("基础宽度", &Config::Collision.fBoneDrivenWidthMultBase, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("潜行宽度", &Config::Collision.fBoneDrivenWidthMultSneaking, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("爬行宽度", &Config::Collision.fBoneDrivenWidthMultCrawling, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("趴伏宽度", &Config::Collision.fBoneDrivenWidthMultProning, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("游泳宽度", &Config::Collision.fBoneDrivenWidthMultSwimming, 0.5f, 3.0f, TWidth, "%.2fx");

				ImGui::Spacing();
			}
		}
	}

	void CategoryCollision::DrawRight() {

		ImUtil_Unique
		{

			PSString TWidth = "调整角色碰撞宽度（左/右）相对基础尺寸的倍率。\n"
							  "1.00x = 默认宽度。较高值会让碰撞体更宽，较低值会更窄。";

			PSString THeight = "调整角色碰撞高度（上/下）相对基础尺寸的倍率。\n"
				               "1.00x = 站立高度。";

			PSString TMax = "简单（非骨骼驱动）碰撞形状允许的最大体型。\n\n"
							"注意：NPC 移动基于导航网格，而不是真正的物理导航。非常大的碰撞体会增加卡住的概率，\n"
							"或触发物理不稳定（可能导致卡顿）。\n\n"
							"注意 2：此形状也会影响投射物碰撞（例如箭矢、火球）以及近战碰撞（如果未安装 Precision）。\n"
						    "如果不打算使用 GTS NPC，最好保持 1.0；否则建议最大体型约为 50x。";

			PSString TMin = "简单碰撞形状允许的最小体型。\n"
				            "作为安全下限，防止碰撞体过小导致穿模或不稳定行为。";

			if (ImGui::CollapsingHeader("简单碰撞体", ImUtil::HeaderFlagsDefaultOpen)) {

				ImGuiEx::SliderF("基础宽度", &Config::Collision.fSimpleDrivenWidthMultBase, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("潜行宽度", &Config::Collision.fSimpleDrivenWidthMultSneaking, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("爬行宽度", &Config::Collision.fSimpleDrivenWidthMultCrawling, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("趴伏宽度", &Config::Collision.fBoneDrivenWidthMultCrawling, 0.5f, 3.0f, TWidth, "%.2fx");
				ImGuiEx::SliderF("游泳宽度", &Config::Collision.fBoneDrivenWidthMultSwimming, 0.5f, 3.0f, TWidth, "%.2fx");

				ImGuiEx::SliderF("最大体型", &Config::Collision.fMSimpleDrivenColliderMaxScale, Config::Collision.fMSimpleDrivenColliderMinScale, 250.0f, TMax, "%.2fx");
				ImGuiEx::SliderF("最小体型", &Config::Collision.fMSimpleDrivenColliderMinScale, 0.05f, Config::Collision.fMSimpleDrivenColliderMaxScale, TMin, "%.2fx");

				ImGuiEx::SliderF("游泳高度", &Config::Collision.fSimpleDrivenHeightMultSwimming, 0.1f, 1.0f, THeight, "%.2fx");
				ImGuiEx::SliderF("潜行高度", &Config::Collision.fSimpleDrivenHeightMultSneaking, 0.1f, 1.0f, THeight, "%.2fx");
				ImGuiEx::SliderF("爬行高度", &Config::Collision.fSimpleDrivenHeightMultCrawling, 0.1f, 1.0f, THeight, "%.2fx");

				ImGui::Spacing();
			}

		}
	}
}
