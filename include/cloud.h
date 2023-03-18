#pragma once
#include "chunk.h"
#include "imgui.h"

// Used externally
constexpr float default_texture_scale = 8.0f;

struct CloudSettings
{
    // Bounds
    float size = Chunk::size * 16.0f;
    float height_min = 60.0f;
    float height_max = 150.0f;
    float scale = 0.083f;

    // Time
    float time = 0.0f;

    // Shading
    float detail_scale = 1.2f;
    float density = 10.0f;
    float threshold = 0.75f;
    float brightness = 11.0f;
    float texture_scale = default_texture_scale;

    // Quality
    int steps = 256;

    bool draw_debug_gui()
    {
        ImGui::Begin("Clouds");
        ImGui::SliderFloat("Scale", &scale, 0.0f, 10.0f);
        ImGui::SliderFloat("Detail scale", &detail_scale, 0.0f, 10.0f);
        ImGui::SliderFloat("Density", &density, 0.0f, 30.0f);
        ImGui::SliderFloat("Threshold", &threshold, 0.0f, 1.0f);
        ImGui::SliderFloat("Brightness", &brightness, 0.0f, 20.0f);
        ImGui::SliderFloat("Texture scale", &texture_scale, 0.0f, 10.0f);
        ImGui::SliderInt("Steps", &steps, 1, 1024);
        bool should_recalculate = ImGui::Button("Recalculate noise");
        ImGui::End();
        return should_recalculate;
    }
};
