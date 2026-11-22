#pragma once

#include "4ZTScout/gui/panel.h"

namespace Scout {

class InspectorPanel : public Panel {
public:
    InspectorPanel() : Panel("Inspector") {}

protected:
    void OnRender(Context& context) override {
        ImGui::TextDisabled("Select an object to inspect it.");
    }
};

} // namespace Scout
