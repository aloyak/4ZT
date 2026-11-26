#pragma once

#include "4ZT/render/gpu-vulkan/vulkan_deps.h"

#include <cstdint>

namespace FourZT {

class VulkanPipeline {
public:
    VulkanPipeline() = default;
    ~VulkanPipeline() { Destroy(); }
    VulkanPipeline(const VulkanPipeline&) = delete;
    VulkanPipeline& operator=(const VulkanPipeline&) = delete;

    void Create(VkDevice device);
    void Destroy();

    VkPipeline       GetPipeline() const       { return m_pipeline; }
    VkPipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
    VkDescriptorSet  GetDescriptorSet() const  { return m_descSet; }

private:
    VkDevice              m_device         = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_descLayout     = VK_NULL_HANDLE;
    VkPipelineLayout      m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline            m_pipeline       = VK_NULL_HANDLE;
    VkDescriptorPool      m_descPool       = VK_NULL_HANDLE;
    VkDescriptorSet       m_descSet        = VK_NULL_HANDLE;
};

} // namespace FourZT