#include "4ZTScout/gui/gui.h"
#include "4ZTScout/gui/panels/scene.h"
#include "4ZTScout/gui/panels/console.h"
#include "4ZTScout/gui/panels/hierarchy.h"
#include "4ZTScout/gui/panels/inspector.h"
#include "4ZTScout/gui/panels/output.h"

#include "4ZTScout/scout.h"
#include "4ZTScout/platform/platform.h"
#include "4ZTScout/platform/window.h"
#include "4ZTScout/render/renderer.h"
#include "4ZTScout/render/vk_instance.h"
#include "4ZTScout/render/swapchain.h"
#include "4ZTScout/filesystem.h"

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include <cstdio>
#include <utility>

void Scout::GUI::OnAttach(Context& context) {
    if (!context.window || !context.renderer) return;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    const std::string mediumFont = FileSystem::Resolve("scout://resources/fonts/CascadiaCode-Medium.ttf").string();
    const std::string boldFont = FileSystem::Resolve("scout://resources/fonts/CascadiaCode-Bold.ttf").string();
    io.FontDefault = io.Fonts->AddFontFromFileTTF(mediumFont.c_str(), 16.0f);
    if (io.FontDefault == nullptr) {
        std::fprintf(stderr, "Failed to load UI font: %s\n", mediumFont.c_str());
    }
    if (io.Fonts->AddFontFromFileTTF(boldFont.c_str(), 16.0f) == nullptr) {
        std::fprintf(stderr, "Failed to load bold UI font: %s\n", boldFont.c_str());
    }

    if (!ImGui_ImplSDL3_InitForVulkan(context.window->GetHandle())) {
        std::fprintf(stderr, "ImGui SDL3 initialization failed\n");
        return;
    }

    VulkanInstance& vk = context.renderer->GetVulkanInstance();
    Swapchain& swapchain = context.renderer->GetSwapchain();

    VkDescriptorPoolSize poolSizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
    };
    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = 1000,
        .poolSizeCount = 1,
        .pPoolSizes = poolSizes,
    };
    if (vkCreateDescriptorPool(vk.GetDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS) {
        std::fprintf(stderr, "ImGui descriptor pool creation failed\n");
        return;
    }

    VkFormat colorFormat = swapchain.Format();
    ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
    pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    pipelineInfo.PipelineRenderingCreateInfo = VkPipelineRenderingCreateInfoKHR{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorFormat,
    };

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.ApiVersion = VK_API_VERSION_1_3;
    initInfo.Instance = vk.GetInstance();
    initInfo.PhysicalDevice = vk.GetPhysicalDevice();
    initInfo.Device = vk.GetDevice();
    initInfo.QueueFamily = vk.GetQueueFamily();
    initInfo.Queue = vk.GetQueue();
    initInfo.DescriptorPool = m_descriptorPool;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = static_cast<uint32_t>(2);
    initInfo.PipelineInfoMain = pipelineInfo;
    initInfo.UseDynamicRendering = true;

    if (!ImGui_ImplVulkan_Init(&initInfo)) {
        std::fprintf(stderr, "ImGui Vulkan initialization failed\n");
        return;
    }

    AddPanel<ScenePanel>();
    AddPanel<HierarchyPanel>();
    AddPanel<InspectorPanel>();
    AddPanel<RenderPanel>();
    AddPanel<ConsolePanel>();

    m_initialized = true;
    context.gui = this;
}

void Scout::GUI::OnDetach(Context& context) {
    if (m_initialized) {
        VkDevice device = context.renderer ? context.renderer->GetVulkanInstance().GetDevice()
                                           : VK_NULL_HANDLE;
        if (device) vkDeviceWaitIdle(device);
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();
        m_initialized = false;
    }
    if (context.renderer && m_descriptorPool) {
        vkDestroyDescriptorPool(context.renderer->GetVulkanInstance().GetDevice(),
                                m_descriptorPool, nullptr);
    }
    m_descriptorPool = VK_NULL_HANDLE;
    context.gui = nullptr;
}

void Scout::GUI::OnUpdate(float dt, Context& context) {
    (void)dt;
    if (!m_initialized || !context.platform) return;

    for (const SDL_Event& event : context.platform->GetEvents())
        ImGui_ImplSDL3_ProcessEvent(&event);

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(),
                                 ImGuiDockNodeFlags_PassthruCentralNode);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                if (context.running) *context.running = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            for (const auto& panel : m_panels)
                ImGui::MenuItem(panel->GetName().c_str(), nullptr, &panel->Open());
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Render")) {
            if (ImGui::MenuItem("Render Scene")) {
                for (const auto& panel : m_panels) {
                    if (panel->GetName() == "Render") {
                        panel->Open() = true;
                        ImGui::SetWindowFocus("Render");
                        break;
                    }
                }
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }

    for (const auto& panel : m_panels)
        panel->Render(context);

    ImGui::Render();
}

void Scout::GUI::RecordDrawData(VkCommandBuffer commandBuffer) {
    if (!m_initialized) return;
    ImDrawData* drawData = ImGui::GetDrawData();
    if (drawData)
        ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer);
}
