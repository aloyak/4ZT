#pragma once

#include "4ZTScout/gui/panel.h"
#include "4ZTScout/filesystem.h"

#include <4ZT/settings.h>
#include <4ZT/output/image.h>
#include <4ZT/output/video.h>

#include <thread>
#include <string>

namespace Scout {

class RenderPanel : public Panel {
public:
    RenderPanel() : Panel("Render") {}

protected:
    void OnRender(Context& context) override {
        ImGui::TextUnformatted("Output settings");
        ImGui::Separator();
        ImGui::Spacing();

        // disabled input text
        ImGui::BeginDisabled();
        ImGui::InputText("Scene", "Active Scene", 256);
        ImGui::EndDisabled();

        // output file
        ImGui::InputText("File", m_outputFile, sizeof(m_outputFile));
        ImGui::InputText("Directory", m_outputDirectory, sizeof(m_outputDirectory));

        if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen)) {
            const char* backends[] = { "CPU", "GPU" };

            if (ImGui::BeginCombo("Backend", backends[m_backend])) {
                for (int i = 0; i < IM_ARRAYSIZE(backends); ++i) {
                    const bool selected = m_backend == i;
                    if (ImGui::Selectable(backends[i], selected))
                        m_backend = i;
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            if (m_backend == 0) {
                ImGui::InputInt("CPU threads", &m_cpuThreads);
                m_settings.targetThreads = m_cpuThreads;
            } else {
                ImGui::InputInt("GPU device", &m_gpuDevice);
            }
        }
        if (ImGui::CollapsingHeader("Ray Tracing", nullptr)) {
            if (ImGui::BeginTable("raytracing_settings", 2, ImGuiTableFlags_BordersInnerH)) {
                ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Samples Per Pixel");
                ImGui::TableSetColumnIndex(1);
                ImGui::InputInt("##samples_per_pixel", &m_settings.samplesPerPixel);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Max Depth");
                ImGui::TableSetColumnIndex(1);
                ImGui::InputInt("##max_depth", &m_settings.maxDepth);

                ImGui::EndTable();
            }
        }
        if (ImGui::CollapsingHeader("Image", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("output_settings", 2, ImGuiTableFlags_BordersInnerH)) {
                ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Output Format");
                ImGui::TableSetColumnIndex(1);
                const char* formats[] = { "PNG", "JPEG", "PPM", "HDR (HDR)", "EXR (HDR)" };
                if (ImGui::BeginCombo("##format", formats[0])) {
                    for (int i = 0; i < IM_ARRAYSIZE(formats); ++i) {
                        const bool selected = i == 0;
                        if (ImGui::Selectable(formats[i], selected))
                            ; // do nothing for now
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Output Height");
                ImGui::TableSetColumnIndex(1);
                ImGui::InputInt("##output_height", &m_settings.outputHeight);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Output Width");
                ImGui::TableSetColumnIndex(1);
                ImGui::InputInt("##aspect_ratio", &m_settings.outputWidth);
                m_settings.aspectRatio = static_cast<double>(m_settings.outputWidth) / m_settings.outputHeight;
                ImGui::EndTable();
            }
        }
        if (ImGui::CollapsingHeader("Video", ImGuiTreeNodeFlags_DefaultOpen)) {
            if (ImGui::BeginTable("video_settings", 2, ImGuiTableFlags_BordersInnerH)) {
                ImGui::TextUnformatted("No video is active in the scene!");
                ImGui::BeginDisabled();
                ImGui::TableSetupColumn("Setting", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Value",   ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableHeadersRow();

                // output format (mp4, mkv, gif)
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Output Format");
                ImGui::TableSetColumnIndex(1);
                const char* formats[] = { "MP4", "MKV", "GIF" };
                if (ImGui::BeginCombo("##format", formats[0])) {
                    for (int i = 0; i < IM_ARRAYSIZE(formats); ++i) {
                        const bool selected = i == 0;
                        if (ImGui::Selectable(formats[i], selected))
                            ; // do nothing for now
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }  

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Video FPS");
                ImGui::TableSetColumnIndex(1);
                ImGui::InputInt("##fps", &m_videoSettings.fps);

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Video Quality");
                ImGui::TableSetColumnIndex(1);
                ImGui::InputInt("##quality", &m_videoSettings.quality); 

                // Codec
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("Video Codec");
                ImGui::TableSetColumnIndex(1);
                const char* codecs[] = { "Auto", "H264", "HEVC", "AV1", "VP9" };
                if (ImGui::BeginCombo("##codec", codecs[static_cast<int>(m_videoSettings.codec)])) {
                    for (int i = 0; i < IM_ARRAYSIZE(codecs); ++i) {
                        const bool selected = static_cast<int>(m_videoSettings.codec) == i;
                        if (ImGui::Selectable(codecs[i], selected))
                            m_videoSettings.codec = static_cast<FourZT::Codec>(i);
                        if (selected)
                            ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }

                ImGui::EndTable();
                ImGui::EndDisabled();
            }
        }

        ImGui::Spacing();
        ImGui::SetCursorPosY(ImGui::GetWindowHeight() - ImGui::GetFrameHeightWithSpacing());
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.2f, 0.4f, 1.0f));
        ImGui::Button("Render!", ImVec2(-1.0f, 0.0f));
        ImGui::PopStyleColor();
    }

private:
    FourZT::Settings m_settings;
    FourZT::VideoSettings m_videoSettings;

    int m_backend = 0;
    int m_cpuThreads = std::thread::hardware_concurrency() - 2;
    int m_gpuDevice = 0;

    char m_outputFile[256] = "output.png";
    char m_outputDirectory[256] = "./";
};

} // namespace Scout
