#include "4ZT/render/gpu-vulkan/vulkan_pipeline.h"
#include "4ZT/render/gpu-vulkan/gpu_types.h"
#include "4ZT/filesystem.h"

#include <vector>
#include <fstream>

// helper function to load SPIR-V binary from file
std::vector<uint32_t> LoadSpirv(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) throw std::runtime_error("4ZT: cannot open shader " + path);
    size_t size = static_cast<size_t>(f.tellg());
    if (size == 0 || size % 4 != 0) throw std::runtime_error("4ZT: invalid SPIR-V " + path);
    std::vector<uint32_t> code(size / 4);
    f.seekg(0);
    f.read(reinterpret_cast<char*>(code.data()), size);
    return code;
}

void FourZT::VulkanPipeline::Create(VkDevice device) {
    m_device = device;

    // the 3 buffers
    // 3 bindings: 0=output buffer, 1=primitive buffer, 2=material buffer
    VkDescriptorSetLayoutBinding bindings[3]{};
    for (uint32_t i = 0; i < 3; ++i) {
        bindings[i].binding         = i;
        bindings[i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags      = VK_SHADER_STAGE_COMPUTE_BIT;
    }
    VkDescriptorSetLayoutCreateInfo dsl{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
    dsl.bindingCount = 3;
    dsl.pBindings    = bindings;
    vkcheck(vkCreateDescriptorSetLayout(m_device, &dsl, nullptr, &m_descLayout));

    // push constants
    VkPushConstantRange pcr{ VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(PushConstants) };
    VkPipelineLayoutCreateInfo pl{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
    pl.setLayoutCount         = 1;
    pl.pSetLayouts            = &m_descLayout;
    pl.pushConstantRangeCount = 1;
    pl.pPushConstantRanges    = &pcr;
    vkcheck(vkCreatePipelineLayout(m_device, &pl, nullptr, &m_pipelineLayout));

    auto code = LoadSpirv(FileSystem::Resolve("shaders://shaders/pathtrace.spv").string());
    VkShaderModuleCreateInfo sm{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
    sm.codeSize = code.size() * sizeof(uint32_t);
    sm.pCode    = code.data();
    VkShaderModule module = VK_NULL_HANDLE;
    vkcheck(vkCreateShaderModule(m_device, &sm, nullptr, &module));

    // the compute pipeline itself
    VkComputePipelineCreateInfo ci{ VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO };
    ci.stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    ci.stage.stage  = VK_SHADER_STAGE_COMPUTE_BIT;
    ci.stage.module = module;
    ci.stage.pName  = "main";
    ci.layout       = m_pipelineLayout;
    VkResult r = vkCreateComputePipelines(m_device, VK_NULL_HANDLE, 1, &ci, nullptr, &m_pipeline); // this compiles the shader, slow
    vkDestroyShaderModule(m_device, module, nullptr);   // not needed after pipeline creation
    vkcheck(r);

    // one pool, one set, reused every Render() call
    VkDescriptorPoolSize ps{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 };
    VkDescriptorPoolCreateInfo dp{ VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
    dp.maxSets       = 1;
    dp.poolSizeCount = 1;
    dp.pPoolSizes    = &ps;
    vkcheck(vkCreateDescriptorPool(m_device, &dp, nullptr, &m_descPool));

    VkDescriptorSetAllocateInfo ai{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
    ai.descriptorPool     = m_descPool;
    ai.descriptorSetCount = 1;
    ai.pSetLayouts        = &m_descLayout;
    vkcheck(vkAllocateDescriptorSets(m_device, &ai, &m_descSet));
}

void FourZT::VulkanPipeline::Destroy() {
    if (m_device == VK_NULL_HANDLE) return;

    if (m_pipeline)       vkDestroyPipeline(m_device, m_pipeline, nullptr);
    if (m_pipelineLayout) vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);
    if (m_descPool)       vkDestroyDescriptorPool(m_device, m_descPool, nullptr); // also frees m_descSet
    if (m_descLayout)     vkDestroyDescriptorSetLayout(m_device, m_descLayout, nullptr);

    m_pipeline = VK_NULL_HANDLE;
    m_pipelineLayout = VK_NULL_HANDLE;
    m_descPool = VK_NULL_HANDLE;
    m_descSet = VK_NULL_HANDLE;
    m_descLayout = VK_NULL_HANDLE;
    m_device = VK_NULL_HANDLE;
}