#pragma once
#include "Gui.h"
#include "imgui.h"
#include <map>

static inline std::string toLower(std::string text) {
    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) { return (char)std::tolower(c); }
    );
    return text;
}

template <typename ItemType, typename Setter>
void Gui::drawListing(
    const std::string& calling,
    const std::vector<std::unique_ptr<ItemType>>& items,
    const ItemType* selected_item,
    Setter&& selectedItemSetter
) {
    static char filter[128] = "";
    ImGui::SetNextItemWidth(-ImGui::GetFrameHeight());
    ImGui::InputTextWithHint("##filter", "Filter ...", filter, sizeof(filter));
    ImGui::SameLine();
    if(ImGui::Button("X", ImVec2(ImGui::GetFrameHeight(), 0.0f))) {
        filter[0] = '\0';
    }

    ImGui::Separator();

    if(filter[0] == '\0') {
        ImGui::TextDisabled("%zu %s(s)", items.size(), calling.c_str());
    }

    std::string needle = toLower(filter);
    auto matches = [&needle](const std::string& name) {
        return needle.empty() || toLower(name).find(needle) != std::string::npos;
    };

    std::map<std::string, std::vector<std::pair<size_t, ItemType*>>> groups;
    for(size_t i = 0; i < items.size(); i++) {
        ItemType& item = *items[i];
        groups[getTypeName(typeid(item))].push_back({i, items[i].get()});
    }

    for(auto& [type, entries] : groups) {
        bool has_visible_entry = false;
        for(const auto& [index, item] : entries) {
            if(matches(item->getName())) {
                has_visible_entry = true;
                break;
            }
        }
        if(!has_visible_entry) continue;

        ImGui::PushID(type.c_str());
        char type_label[160];
        std::snprintf(type_label, sizeof(type_label), "%s (%zu)", type.c_str(), entries.size());

        if(ImGui::TreeNodeEx(
            type_label,
            ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_SpanAvailWidth
        )) {
            for(const auto& [index, item] : entries) {
                if(!matches(item->getName())) continue;

                char label[160];
                std::snprintf(
                    label,
                    sizeof(label),
                    "[%zu] %s",
                    index,
                    item->getName().c_str()
                );

                if(ImGui::Selectable(label, item == selected_item)) {
                    selectedItemSetter(item, true);
                }
            }
            ImGui::TreePop();
        }
        ImGui::PopID();
    }
}
