#include "4ZT/render/gpu-vulkan/vulkan_renderer.h"
#include "4ZT/render/gpu-vulkan/vulkan_buffer.h"
#include "4ZT/render/gpu-vulkan/gpu_scene.h"

#include <cmath>
#include <iomanip>
#include <iostream>
#include <chrono>

FourZT::VulkanRenderer::VulkanRenderer(const Settings& settings) {
    m_context.Initialize(settings);
    m_pipeline.Create(m_context.GetDevice());

    /// TODO: this may cause off-by-one errors if doubles get truncated, also on camera.h
    m_imgHeight = settings.outputHeight;
    m_imgWidth  = static_cast<int>(std::lround(settings.outputHeight * settings.aspectRatio));
}

FourZT::Image FourZT::VulkanRenderer::Render(const Settings& settings,
                                             const Camera& camera,
                                             const World& world) {
    Image img(m_imgWidth, m_imgHeight);
    ViewportInfo vp = Camera::ComputeViewport(camera, settings);
    VkDevice device = m_context.GetDevice();

    // scene buffers
    GpuSceneData scene = BuildGpuScene(world);
    const uint32_t primCount = uint32_t(scene.primitives.size());
    if (scene.primitives.empty()) scene.primitives.emplace_back(); // Vulkan forbids zero-size buffers apparently
    if (scene.materials.empty())  scene.materials.emplace_back();

    const VkDeviceSize primBytes = scene.primitives.size() * sizeof(GpuPrimitive);
    const VkDeviceSize matBytes  = scene.materials.size()  * sizeof(GpuMaterial);
    const VkDeviceSize outBytes  = VkDeviceSize(m_imgWidth) * m_imgHeight * 4 * sizeof(float);

    GpuBuffer primBuf(m_context.GetAllocator(), primBytes, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
    GpuBuffer matBuf(m_context.GetAllocator(), matBytes,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
    GpuBuffer out(m_context.GetAllocator(), outBytes,  VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, true);
    primBuf.Upload(scene.primitives.data(), primBytes);
    matBuf.Upload(scene.materials.data(),  matBytes);

    // a default buffer is full of random garbage
    // fills the output buffer with zeros so that the first batch can accumulate into it
    // not needed for the other two buffers, they completely overwrite their contents
    std::memset(out.mapped(), 0, size_t(outBytes));
    vmaFlushAllocation(m_context.GetAllocator(), out.allocation(), 0, VK_WHOLE_SIZE);

    // descriptors 
    VkDescriptorBufferInfo infos[3] = {
        { out.handle(),     0, VK_WHOLE_SIZE },   // binding 0: output
        { primBuf.handle(), 0, VK_WHOLE_SIZE },   // binding 1: primitives
        { matBuf.handle(),  0, VK_WHOLE_SIZE },   // binding 2: materials
    };
    VkWriteDescriptorSet writes[3]{};
    for (uint32_t i = 0; i < 3; ++i) {
        writes[i].sType           = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet          = m_pipeline.GetDescriptorSet();
        writes[i].dstBinding      = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        writes[i].pBufferInfo     = &infos[i];
    }
    vkUpdateDescriptorSets(device, 3, writes, 0, nullptr);

    // push constants (everything except the per-batch sample range)
    auto Float4 = [](float (&d)[4], const Vec3& v) {
        d[0] = float(v.x); d[1] = float(v.y); d[2] = float(v.z); d[3] = 0.f;
    };

    // give the static settings and camera info to the shader
    PushConstants pc{};
    Float4(pc.origin,     vp.origin);
    Float4(pc.pixel00,    vp.pixel00);
    Float4(pc.deltaU,     vp.deltaU);
    Float4(pc.deltaV,     vp.deltaV);
    Float4(pc.background, camera.background);
    pc.width     = uint32_t(m_imgWidth);
    pc.height    = uint32_t(m_imgHeight);
    pc.primCount = primCount;   // the real count, not the dummy-padded size
    pc.maxDepth  = uint32_t(settings.maxDepth);
    pc.minDepth  = uint32_t(settings.minDepth);
    pc.useSkybox = camera.useSkybox ? 1u : 0u;
    pc.enableRR  = settings.enableRussianRoulette ? 1u : 0u;

    // command buffer + fence, reused for every batch
    VkCommandBufferAllocateInfo cai{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
    cai.commandPool        = m_context.GetCommandPool();
    cai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    cai.commandBufferCount = 1;
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    vkcheck(vkAllocateCommandBuffers(device, &cai, &cmd));

    VkFenceCreateInfo fi{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
    VkFence fence = VK_NULL_HANDLE;
    vkcheck(vkCreateFence(device, &fi, nullptr, &fence));

    VkDescriptorSet set = m_pipeline.GetDescriptorSet();
    const uint32_t total = std::max(1, settings.samplesPerPixel);

    // Each dispatch must stay well under the driver watchdog (a few seconds).
    // Start with 1 sample, then size every next batch to take about 0.25 s.
    constexpr double   kTargetSeconds = 0.25;
    constexpr uint32_t kMaxBatch      = 64;
    uint32_t batch = 1; // spp per dispatch, adjusted dynamically later
    // if compute takes longer than the watchdog's limit (2-3 seconds), the driver will likely
    // terminate the process, so we gotta target somewhere around .25-.5 seconds per dispatch
    // and upload overhead is negligible even then

    try {
        for (uint32_t start = 0; start < total; ) {
            pc.sampleStart = start;
            pc.sampleCount = std::min(batch, total - start);

            auto t0 = std::chrono::steady_clock::now();

            vkcheck(vkResetCommandBuffer(cmd, 0));
            VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
            bi.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
            vkcheck(vkBeginCommandBuffer(cmd, &bi));

            vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipeline.GetPipeline());
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE,
                                    m_pipeline.GetPipelineLayout(), 0, 1, &set, 0, nullptr);
            vkCmdPushConstants(cmd, m_pipeline.GetPipelineLayout(), VK_SHADER_STAGE_COMPUTE_BIT,
                               0, sizeof(pc), &pc);
            vkCmdDispatch(cmd, (m_imgWidth + 7) / 8, (m_imgHeight + 7) / 8, 1);

            // writes visible to the host (readback) and to the next batch (+= on the output)
            VkMemoryBarrier2 mb{ VK_STRUCTURE_TYPE_MEMORY_BARRIER_2 };
            mb.srcStageMask  = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            mb.srcAccessMask = VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            mb.dstStageMask  = VK_PIPELINE_STAGE_2_HOST_BIT | VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
            mb.dstAccessMask = VK_ACCESS_2_HOST_READ_BIT
                             | VK_ACCESS_2_SHADER_STORAGE_READ_BIT
                             | VK_ACCESS_2_SHADER_STORAGE_WRITE_BIT;
            VkDependencyInfo dep{ VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
            dep.memoryBarrierCount = 1;
            dep.pMemoryBarriers    = &mb;
            vkCmdPipelineBarrier2(cmd, &dep);
            vkcheck(vkEndCommandBuffer(cmd));

            VkSubmitInfo si{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
            si.commandBufferCount = 1;
            si.pCommandBuffers    = &cmd;
            vkcheck(vkResetFences(device, 1, &fence));
            vkcheck(vkQueueSubmit(m_context.GetQueue(), 1, &si, fence));
            vkcheck(vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX));

            // here we see how long it took the batch to run, we can use this information to
            // size the next batch to take the target amount of time
            double sec = std::chrono::duration<double>(std::chrono::steady_clock::now() - t0).count();
            double perSample = sec / pc.sampleCount;
            batch = std::clamp(uint32_t(kTargetSeconds / std::max(perSample, 1e-6)), 1u, kMaxBatch);

            start += pc.sampleCount;
            std::cerr << "\rRendering: " << std::fixed << std::setprecision(1)
                      << (100.0 * start / total) << "% " << std::flush;
        }
    } catch (...) {
        vkDeviceWaitIdle(device);
        vkDestroyFence(device, fence, nullptr);
        vkFreeCommandBuffers(device, m_context.GetCommandPool(), 1, &cmd);
        std::cerr << std::endl;
        throw;
    }
    std::cerr << "\rComplete.               " << std::endl;

    vkDestroyFence(device, fence, nullptr);
    vkFreeCommandBuffers(device, m_context.GetCommandPool(), 1, &cmd);

    // copy to the image, dividing by the total sample count
    vmaInvalidateAllocation(m_context.GetAllocator(), out.allocation(), 0, VK_WHOLE_SIZE);
    const float  inv = 1.0f / float(total);
    const float* px  = static_cast<const float*>(out.mapped());
    for (int y = 0; y < m_imgHeight; ++y) {
        for (int x = 0; x < m_imgWidth; ++x) {
            const float* p = px + (size_t(y) * m_imgWidth + x) * 4;
            img.at(x, y) = Color(p[0] * inv, p[1] * inv, p[2] * inv);
        }
    }
    return img;
}