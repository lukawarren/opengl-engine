#pragma once
#include "window.h"
#include "shader.h"
#include "entity.h"
#include "camera.h"
#include "water.h"
#include "framebuffer.h"
#include <string>
#include <array>

class Renderer
{
public:
    Renderer(const std::string& title, const int width, const int height);
    ~Renderer();

    bool update(const std::vector<Entity>& entities, const std::vector<Water>& waters,
        const Camera& camera);

private:
    Window window;

    DiffuseShader diffuse_shader;
    WaterShader water_shader;
    std::array<Shader*, 2> shaders = { &diffuse_shader, &water_shader };

    Framebuffer framebuffer;
};