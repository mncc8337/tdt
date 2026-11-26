#pragma once
#include "Gui.h"
#include "imgui.h"
#include "misc/NamedElement.h"
#include <map>

template <typename Item, typename OnChanged>
void Gui::drawResourceCombo(
    const char* label,
    const std::vector<std::unique_ptr<Item>>& items,
    const Item* current,
    OnChanged&& onChanged
) {
    if(items.empty()) {
        ImGui::TextDisabled("No %s available.", label);
        return;
    }

    // ids keep duplicate names distinguishable in the combo
    std::vector<std::string> labels;
    labels.reserve(items.size());
    for(const auto& item : items) {
        labels.push_back(
            std::string(item->getName()) + " #" + std::to_string(item->getID())
        );
    }

    int selected = 0;
    for(size_t i = 0; i < items.size(); i++) {
        if(items[i].get() == current) {
            selected = (int)i;
            break;
        }
    }

    auto itemGetter = [](void* data, int index, const char** out) {
        const auto& strings = *static_cast<const std::vector<std::string>*>(data);
        *out = strings[index].c_str();
        return true;
    };

    if(ImGui::Combo(
        label,
        &selected,
        itemGetter,
        &labels,
        (int)labels.size()
    )) {
        onChanged(items[selected].get());
    }
}

static inline std::string toLower(std::string text) {
    std::transform(
        text.begin(),
        text.end(),
        text.begin(),
        [](unsigned char c) { return (char)std::tolower(c); }
    );
    return text;
}

namespace {
// translation for the generic below
void remove_item(Scene& scene, const Hittable* hittable) {
    scene.remove_object(hittable);
}
void remove_item(Scene& scene, const Material* material) {
    scene.remove_material(material);
}
void remove_item(Scene& scene, const Texture* texture) {
    scene.remove_texture(texture);
}
void remove_item(Scene& scene, const Medium* medium) {
    scene.remove_medium(medium);
}
} // namespace

template <typename ItemType, typename Setter>
void Gui::drawListing(
    const std::string& calling,
    const std::vector<std::unique_ptr<ItemType>>& items,
    ItemType* selected_item,
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

    if(selected_item) {
        ImGui::Separator();

        NamedElement* named = static_cast<NamedElement*>(selected_item);
        static char item_name[128];
        const std::string& current_name = named->getName();
        std::snprintf(item_name, sizeof(item_name), "%s", current_name.c_str());

        if(ImGui::InputText(
            "##name",
            item_name,
            sizeof(item_name),
            ImGuiInputTextFlags_EnterReturnsTrue
        )) {
            std::string new_name(item_name);
            if(new_name != current_name) {
                postNonResetAction([named, new_name]() {
                    named->setName(new_name);
                });
            }
        }
        if(ImGui::IsItemDeactivatedAfterEdit()) {
            std::string new_name(item_name);
            if(new_name != current_name) {
                postNonResetAction([named, new_name]() {
                    named->setName(new_name);
                });
            }
        }

        if(ImGui::Button("Delete item")) {
            postAction([this, selected_item]() {
                remove_item(scene, selected_item);
                resetSelection();
            });
        }
    }
}
