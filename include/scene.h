#pragma once
#include <vector>
#include "entity.h"
#include "camera.h"
#include "water.h"
#include "light.h"
#include "chunk.h"

struct Scene
{
    // "Renderables"
    std::vector<Chunk> chunks = {};
    std::vector<Entity> entities = {};
    std::vector<Water> waters = {};

    // Lighting
    DirectionalLight sun = {};
    glm::vec3 ambient_light = { 0.1f, 0.1f, 0.1f };

    Camera camera = {};
};
