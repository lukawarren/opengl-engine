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
    void diffuse_pass(
        const Scene& scene,
        const Camera& camera,
        const unsigned int width,
        const unsigned int height,
        const std::optional<glm::vec4> clip_plane = {});
    void water_pass(const Scene& scene);

    unsigned int output_width() const;
    unsigned int output_height() const;
    Framebuffer output_framebuffer;
    float render_scale;

    DiffuseShader diffuse_shader;
    WaterShader water_shader;
    QuadShader quad_shader;
    TerrainShader terrain_shader;
};