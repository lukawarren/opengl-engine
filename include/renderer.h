#pragma once
#include "window.h"
#include "shader.h"
#include "entity.h"
#include "camera.h"
#include <string>

class Renderer
{
public:
    Renderer(const std::string& title, const int width, const int height);
    bool update(const std::vector<Entity>& entities, const Camera& camera);

private:
    Window window;
    DiffuseShader diffuse_shader;
};