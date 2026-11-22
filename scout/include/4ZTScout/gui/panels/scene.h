#pragma once

#include "4ZTScout/gui/panel.h"

namespace Scout {

class ScenePanel : public Panel {
public:
    ScenePanel() : Panel("Scene") {}

protected:
    void OnRender(Context& context) override {
        ImGui::TextUnformatted("Realtime preview");
        ImGui::Separator();
        ImGui::TextDisabled("No preview draw calls connected yet.");
    }
};

} // namespace Scout
