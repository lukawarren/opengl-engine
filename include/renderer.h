#pragma once
#include "render_passes/render_passes.h"
#include "window.h"
#include "scene.h"
#include <string>
#include <array>
#include <optional>

class Renderer
{
public:
    Renderer(const std::string& title, const int width, const int height, const float render_scale);
    ~Renderer();

    bool update(Scene& scene);

    Window window;

private:

    // Output
    float render_scale;
    Framebuffer output_framebuffer;
    unsigned int render_width() const;
    unsigned int render_height() const;

    // Render passes
    DiffusePass     diffuse_pass;
    WaterPass       water_pass;
    ShadowPass      shadow_pass;
    BloomPass       bloom_pass;
    SpritePass      sprite_pass;
    CloudPass       cloud_pass;
    CompositePass   composite_pass;

    // Frame state
    bool did_bake_shadows = false;
    double last_fps_report_time = 0.0f;
};