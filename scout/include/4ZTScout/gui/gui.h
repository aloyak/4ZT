#pragma once

#include "4ZTScout/layer.h"
#include "4ZTScout/gui/panel.h"

#include <memory>
#include <utility>
#include <vector>

#include <volk.h>

namespace Scout {

class GUI : public Layer {
public:
    void OnAttach(Context& context) override;
    void OnDetach(Context& context) override;
    void OnUpdate(float dt, Context& context) override;
    void OnRender(Context& context) override {}

    template<typename T, typename... Args>
    T& AddPanel(Args&&... args) {
        auto& panel = *m_panels.emplace_back(std::make_unique<T>(std::forward<Args>(args)...));
        return static_cast<T&>(panel);
    }
    void RecordDrawData(VkCommandBuffer commandBuffer);

private:
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<std::unique_ptr<Panel>> m_panels;
    bool m_initialized = false;
};

} // scout