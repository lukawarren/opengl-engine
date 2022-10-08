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
    Renderer(const std::string& title, const int width, const int height);
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

    DiffuseShader diffuse_shader;
    WaterShader water_shader;
};