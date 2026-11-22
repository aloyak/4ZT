#pragma once

#include "4ZTScout/gui/panel.h"

namespace Scout {

class HierarchyPanel : public Panel {
public:
    HierarchyPanel() : Panel("Hierarchy") {}

protected:
    void OnRender(Context& context) override {
        ImGui::TextUnformatted("Scene objects");
    }
};

} // namespace Scout
