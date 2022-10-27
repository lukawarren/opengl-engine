#pragma once
#include "window.h"
#include "shader.h"
#include "scene.h"
#include <string>
#include <array>
#include <optional>

class Renderer
{
public:
    Renderer(const std::string& title, const int width, const int height, const float render_scale);
    ~Renderer();

    bool update(const Scene& scene);

    Window window;

private:

    // Render passes
    void diffuse_pass(
        const Scene& scene,
        const Camera& camera,
        const unsigned int width,
        const unsigned int height,
        const std::optional<glm::vec4> clip_plane = {});
    void water_pass(const Scene& scene);
    void shadow_pass(const Scene& scene);

    // Output
    float render_scale;
    Framebuffer output_framebuffer;
    unsigned int render_width() const;
    unsigned int render_height() const;

    // Shaders
    DiffuseShader diffuse_shader;
    WaterShader water_shader;
    QuadShader quad_shader;
    TerrainShader terrain_shader;
    ShadowMapShader shadow_shader;

    bool did_bake_shadows = false;
};