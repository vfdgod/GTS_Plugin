#include "UI/Windows/Settings/Categories/Keybinds.hpp"
#include "UI/Windows/Settings/SettingsWindow.hpp"

#include "UI/Controls/Button.hpp"
#include "UI/Controls/CheckBox.hpp"
#include "UI/Controls/ComboBox.hpp"
#include "UI/Controls/Misc.hpp"
#include "UI/Controls/ToolTip.hpp"

#include "UI/Core/ImUtil.hpp"
#include "UI/Core/ImInput.hpp"

#include "Config/Keybinds.hpp"

#include "Config/Config.hpp"

#include "UI/GTSMenu.hpp"
#include "UI/Controls/Text.hpp"

namespace {

    PSString T0 = "禁用此输入事件。\n"
        "禁用的事件会被游戏完全忽略，永远不会触发。";

    PSString T1 = "当动作标记为独占时，只有按下完全匹配的按键组合才会激活。\n"
        "（例如：某动作需要 ALT+E 激活，而你在触发时还按着 W，则在此标记启用时不会发生任何事，除非先松开 W。）";

    PSString T2 = "动作触发类型会改变动作的激活方式。\n\n"
        "- Once：按下按键组合时触发一次动作。\n"
        "- Release：按下后松开按键时才触发动作。\n"
        "- Continuous：只要按住按键组合，每个游戏帧都会触发动作事件。";

    PSString T3 = "通常按下按键组合时，当前按住的按键会同时发送给模组和游戏。\n"
        "根据按键不同，这可能产生不想要的效果，所以需要此选项。\n\n"
        "- Automatic：仅当对应 GTS 动作有效时，阻止游戏读取该动作按键。（例如你拥有相关 Perk/动作当前可执行。）\n"
        "  （注意：部分动作与此设置不兼容，因此默认故意设为 \"Never\"。）\n"
        "- Never：即使动作有效，也永不阻止游戏读取该动作按键。\n"
        "- Always：无论动作是否会触发/执行，都始终阻止游戏读取此按键组合。";

    PSString T4 = "按下按键后，为动作触发添加时间延迟。\n"
        "（例如触发类型为 Once 且此值为 1.0 时，需要按住正确按键组合至少 1 秒，此事件动作才会触发。）";

    PSString T5 = "修改触发此事件的按键组合。\n"
        "创建按键组合时不需要一直按住按键；按下一次就会追加到列表中。\n"
        "输入新的按键组合后，再次点击此按钮即可保存。\n"
        "按 ESC 可取消重新绑定。";

    PSString T6 = "点击打开此按键绑定的高级设置。";


    PSString TH0 = "按动作名称筛选。";
    PSString TH1 = "将所有按键绑定及其设置重置为默认值。";

}

namespace GTS {

    CategoryKeybinds::CategoryKeybinds() {
        m_name = "按键";
        for(auto& e : DefaultEvents){
            HeaderStateMap.emplace(e.Event.Event,false);
        }

        // Build Maps

        categoryMap = [] {
	        absl::flat_hash_map<std::string, size_t> m;
            m.reserve(128);
	        for (auto const& ce : DefaultEvents) {
		        size_t catIndex = magic_enum::enum_index(ce.UICategory).value_or(0);
		        m.emplace(ce.Event.Event, catIndex);
	        }
	        return m;
        }();

        hiddenMap = [] {
	        absl::flat_hash_map<std::string, bool> m;
            m.reserve(128);
	        for (auto const& ce : DefaultEvents) {
		        m.emplace(ce.Event.Event, ce.AdvFeature);
	        }
	        return m;
        }();

        uiNameMap = [] {
	        absl::flat_hash_map<std::string, const char*> m;
            m.reserve(128);
	        for (auto const& ce : DefaultEvents) {
		        m.emplace(ce.Event.Event, ce.UIName);
	        }
	        return m;
        }();

        uiDescriptionMap = [] {
	        absl::flat_hash_map<std::string, const char*> m;
            m.reserve(128);
	        for (auto const& ce : DefaultEvents) {
		        m.emplace(ce.Event.Event, ce.UIDescription);
	        }
	        return m;
        }();

    }

    void CategoryKeybinds::Draw() {

        //New Render Loop. Reset Index.
        CurEventIndex = UINT16_MAX;

        //Calc the correct width
        Width = ImGui::GetContentRegionAvail().x - ((ImGui::GetStyle().CellPadding.x * 2 + ImGui::GetStyle().FramePadding.x * 2) * Div);

        //Draw top bar
        DrawOptions();

        //Draw the list containing each input event.
        DrawContent();
    }

    void CategoryKeybinds::DrawOptions() {

        ImGui::SetNextWindowBgAlpha(0.1f);

        ImGui::BeginChild("Options", { -FLT_MIN, 0.0f }, ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_FrameStyle);
        {
            ImGui::BeginDisabled(RebindIndex > 0);
            {
                {
                    ImGui::InputText("搜索", &SearchRes);
                    ImGuiEx::Tooltip(TH0);
                }

                ImGui::SameLine();

                
                ImGuiEx::CheckBox("紧凑视图", &singleColumn, "在单列中列出所有按键绑定。");
                Div = 1 + singleColumn;

                ImGuiEx::SeperatorV();

                if (ImGuiEx::ImageButton("重置", ImageList::Generic_Reset, 18, TH1)) {
                    Keybinds::ResetKeybinds();
                }

            }
            ImGui::EndDisabled();
        }
        ImGui::EndChild();
    }

    void CategoryKeybinds::DrawContent() {

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(8.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 4.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f, 6.0f));

        ImGui::BeginChild("##InputEvents", ImVec2(0, 0));
        {

            // Check if search matches any category name
            absl::flat_hash_set<size_t> matchedCategories;
            for (size_t i = 0; i < std::size(Strings::Keybinds::CategoryNames); ++i) {
                if (ContainsString(Strings::Keybinds::CategoryNames[i], SearchRes)) {
                    matchedCategories.insert(i);
                }
            }

            absl::flat_hash_map<size_t, std::vector<BaseEventData_t*>> groups;
            groups.reserve(128);

            for (auto& ev : Keybinds::InputEvents) {

                // Check if hidden (skip if not in advanced mode)
                if (!Config::Hidden.IKnowWhatImDoing) {
                    auto hit = hiddenMap.find(ev.Event);
                    if (hit != hiddenMap.end() && hit->second) {
                        continue;
                    }
                }

                // Get UI name
                std::string uiName;
                auto nameIt = uiNameMap.find(ev.Event);
                if (nameIt != uiNameMap.end() && nameIt->second != nullptr && nameIt->second[0] != '\0') {
                    uiName = nameIt->second;
                } else {
                    uiName = HumanizeString(ev.Event);
                }

                // find its category (default to 0 if missing)
                auto it = categoryMap.find(ev.Event);
                size_t catIndex = (it != categoryMap.end()) ? it->second : 0;

                // filter by search - check if search matches the event name OR its category
                bool matchesEvent = ContainsString(uiName, SearchRes);
                bool matchesCategory = matchedCategories.contains(catIndex);

                if (!matchesEvent && !matchesCategory) {
                    continue;
                }

                groups[catIndex].push_back(&ev);
            }

            // Measure widest name across ALL events globally (do this once before any category loop)
            std::vector GlobalColumnNameWidths(Div, 0.0f);
            for (auto& evt : Keybinds::InputEvents) {
                // Get UI name for width calculation
                std::string displayName;
                auto nameIt = uiNameMap.find(evt.Event);
                if (nameIt != uiNameMap.end() && nameIt->second != nullptr && nameIt->second[0] != '\0') {
                    displayName = nameIt->second;
                } else {
                    displayName = HumanizeString(evt.Event);
                }

                ImGui::PushFont(nullptr, 21.0f);
                {
                    float w = ImGui::CalcTextSize(displayName.c_str()).x;
                    for (int c = 0; c < Div; ++c) {
                        GlobalColumnNameWidths[c] = std::max(GlobalColumnNameWidths[c], w);
                    }
                }
                ImGui::PopFont();
            }

            for (size_t catIndex = 0; catIndex < std::size(Strings::Keybinds::CategoryNames); ++catIndex) {

                auto git = groups.find(catIndex);
                if (git == groups.end() || git->second.empty()) {
                    continue;
                }

                const std::string catName = Strings::Keybinds::CategoryNames[catIndex];

                ImGui::PushFont(nullptr, 24.0f);
                {
                    ImGui::SeparatorText(catName.c_str());
                }
                ImGui::PopFont();

                // split into Div columns
                auto& list = git->second;
                std::vector<std::vector<BaseEventData_t*>> Columns(Div);
                Columns.reserve(16);
                for (size_t i = 0; i < list.size(); ++i) {
                    Columns[i % Div].push_back(list[i]);
                }

                // render child for each column
                for (int c = 0; c < Div; ++c) {

                    if (c > 0) {
                        ImGuiEx::SeperatorV();
                        ImGui::SameLine(0, 24.0f);
                    }

                    ImGui::BeginChild(static_cast<ImGuiID>(__COUNTER__ | static_cast<int>(catIndex) << 4 | c), { Width / Div, 0 },  ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);
                    {

                        for (auto* ptr : Columns[c]) {
                            // Get UI name and description
                            std::string displayName;
                            auto nameIt = uiNameMap.find(ptr->Event);
                            if (nameIt != uiNameMap.end() && nameIt->second != nullptr && nameIt->second[0] != '\0') {
                                displayName = nameIt->second;
                            } else {
                                displayName = HumanizeString(ptr->Event);
                            }

                            std::string description;
                            auto descIt = uiDescriptionMap.find(ptr->Event);
                            if (descIt != uiDescriptionMap.end()) {
                                description = descIt->second;
                            }

                            DrawInputEvent(*ptr, displayName, description.c_str(), GlobalColumnNameWidths[c]);
                        }
                    }
                    ImGui::EndChild();
                }

            }

            // If nothing printed at all
            bool any = std::ranges::any_of(groups, [](auto& kv) {
                return !kv.second.empty();
            });

            if (!any) {
                ImGui::Text("没有匹配搜索文本的结果。");
            }

            ImGui::PopStyleVar(3);
        }
        ImGui::EndChild();
    }

    void CategoryKeybinds::SetWindowBusy(const bool a_busy) {
        if (SettingsWindow* Window = dynamic_cast<SettingsWindow*>(GTSMenu::WindowManager->wSettings)) {
            Window->m_busy = a_busy;
            Window->m_disableUIInteraction = a_busy;
        }
    }

    bool CategoryKeybinds::DrawInputEvent(BaseEventData_t& Event, const std::string& a_name, const char* a_description, float columnNameWidth) {

        const float ButtonImageSize = 18 * ImGui::GetStyle().FontScaleMain;
        const float ButtonSize = ButtonImageSize + ImGui::GetStyle().ItemSpacing.x + (ImGui::GetStyle().FramePadding.x * 2.0f);
        const bool IsRebinding = (RebindIndex == CurEventIndex && RebindIndex != 0);

        // Extra padding between name and key input
        const float nameToKeyPadding = ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x * 2;

        // Get available region width inside the child
        float contentWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().FramePadding.x * 2 - ImGui::GetStyle().CellPadding.x * 2;
        float nameColWidth = columnNameWidth + nameToKeyPadding;

        // Keys column takes remaining space
        float keysColWidth = contentWidth - nameColWidth - (ButtonSize * 2);

        ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(4, 4));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 4));

        ImGui::BeginChild(CurEventIndex, { Width / Div, 0.0f }, ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);
        {
            ImGui::BeginDisabled(RebindIndex != CurEventIndex && RebindIndex != 0);
            {
                ImGui::BeginTable("##KeybindRow", 4, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_NoPadInnerX);
                {
                    // Setup column widths
                    ImGui::TableSetupColumn("##Name", ImGuiTableColumnFlags_WidthFixed, nameColWidth);
                    ImGui::TableSetupColumn("##Keys", ImGuiTableColumnFlags_WidthFixed, keysColWidth);
                    ImGui::TableSetupColumn("##Rebind", ImGuiTableColumnFlags_WidthFixed, ButtonSize);
                    ImGui::TableSetupColumn("##Options", ImGuiTableColumnFlags_WidthFixed, ButtonSize);

                    // Column 1: Action Name
                    ImGui::TableNextColumn();

                    ImGui::PushFont(nullptr, 21.0f);
                    {
                        ImGuiEx::TextColorShadow(Event.Disabled ? ImUtil::Colors::Warning : ImGui::GetStyle().Colors[ImGuiCol_Text] ,"%s", a_name.c_str());
                        if (a_description != nullptr && a_description[0] != '\0') {
                            ImGuiEx::Tooltip(a_description, true);
                        }
                    }
                    ImGui::PopFont();

                    // Column 2: Current Keys
                    ImGui::TableNextColumn();
                    std::string VisualKeyString;
                    auto& VisualKeyList = IsRebinding ? TempKeys : Event.Keys;
                    for (size_t i = 0; i < VisualKeyList.size(); ++i) {
                        VisualKeyString += VisualKeyList[i];
                        if (i + 1 < VisualKeyList.size()) VisualKeyString += " + ";
                    }
                    std::string InputText = VisualKeyString.empty() ? "按任意键，或按 ESC 取消" : VisualKeyString;

                    ImGui::SetNextItemWidth(keysColWidth - ImGui::GetStyle().CellPadding.x * 2);
                    ImGui::BeginDisabled(Event.Disabled);
                    {
                        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                        ImGui::InputText("##KeyRebind", &InputText, ImGuiInputTextFlags_ReadOnly);
                        ImGui::PopItemFlag();
                    }
                    ImGui::EndDisabled();

                    // Column 3: Rebind Button
                    ImGui::TableNextColumn();
                    SetWindowBusy(RebindIndex > 0);

                    ImGui::BeginDisabled((TempKeys.empty() && IsRebinding) || Event.Disabled);
                    {
                        if (ImGuiEx::ImageButton(("##Rebind" + std::to_string(CurEventIndex)).c_str(), IsRebinding ? ImageList::Generic_OK : ImageList::Keybind_EditKeybind, 18, T5)) {
                            RebindIndex = CurEventIndex;
                            if (IsRebinding) {
                                if (!TempKeys.empty()) {
                                    Event.Keys = TempKeys;
                                    TempKeys.clear();
                                }
                                RebindIndex = 0;
                            }
                        }
                    }
                    ImGui::EndDisabled();

                    // Column 4: Options Button
                    ImGui::BeginDisabled(IsRebinding);
                    ImGui::TableNextColumn();
                    if (ImGuiEx::ImageButton(("##OptionsOpen" + std::to_string(CurEventIndex)).c_str(), ImageList::Keybind_ShowAdvanced, 18, T6)) {
                        ImGui::OpenPopup(("##Options" + std::to_string(CurEventIndex)).c_str());
                    }
                    ImGui::EndDisabled();

                    // Options Popup
                    if (ImGui::BeginPopup(("##Options" + std::to_string(CurEventIndex)).c_str())) {
                        ImGui::Text("额外选项：%s", a_name.c_str());
                        ImGui::Separator();
                        ImGui::PushItemWidth(250.0f);
                        ImGuiEx::CheckBox("禁用", &Event.Disabled, T0);
                        if (!Event.Disabled) {
                            ImGui::BeginDisabled(IsRebinding);
                            ImGuiEx::CheckBox("独占", &Event.Exclusive, T1);
                            ImGuiEx::ComboEx<LTriggerType_t>("触发类型", Event.Trigger, T2);
                            ImGuiEx::ComboEx<LBlockInputTypes_t>("阻止输入", Event.BlockInput, T3);
                            ImGui::InputFloat("触发延迟", &Event.Duration, 0.1f, 0.01f, "%.2f 秒");
                            ImGuiEx::Tooltip(T4);
                            Event.Duration = std::clamp(Event.Duration, 0.0f, 10.0f);
                            ImGui::EndDisabled();
                        }
                        ImGui::PopItemWidth();
                        ImGui::EndPopup();
                    }
                }
                ImGui::EndTable();
                ImGui::PopStyleVar(2);

                // Rebind Handling
                if (IsRebinding) {
                    if (ImGui::IsKeyReleased(ImGuiKey_Escape)) {
                        RebindIndex = 0;
                        TempKeys.clear();
                    }
                    else if (!(ImGui::IsAnyItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))) {
                        for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key) {
                            if (key == ImGuiKey_Escape) continue;
                            if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(key))) {
                                auto keyName = ImInput::ImGuiKeyToDIKString(static_cast<ImGuiKey>(key));
                                if (keyName == "INVALID") continue;
                                if (std::ranges::find(TempKeys, keyName) == TempKeys.end()
                                    && TempKeys.size() < 5) {
                                    TempKeys.push_back(keyName);
                                    std::ranges::sort(TempKeys, [](auto& a, auto& b) {
                                        return a.size() > b.size();
                                    });
                                }
                            }
                        }
                    }
                }
                CurEventIndex++;
            }
            ImGui::EndDisabled();
        }
        ImGui::EndChild();
        return true;
    }
}
