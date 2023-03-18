#pragma once
#include <vector>
#include "entity.h"
#include "camera.h"
#include "water.h"
#include "light.h"
#include "chunk.h"
#include "resources.h"
#include "cloud.h"

typedef std::shared_ptr<Texture> Skybox;
#define MAKE_SKYBOX(a, b, c, d, e, f) std::make_shared<Texture>(std::array<std::string, 6> {\
    a, b, c, d, e, f\
})

struct Sprite
{
    Transform transform = {};
    Texture* texture;

    Sprite(const std::string& filename)
    {
        texture = get_texture(filename, true);
    }
};

struct Scene
{
    // "Renderables"
    std::vector<Chunk> chunks = {};
    std::vector<Entity> entities = {};
    std::vector<Water> waters = {};
    std::vector<Sprite> sprites = {};
    std::optional<Skybox> skybox = {};

    // Lighting
    DirectionalLight sun = {};
    glm::vec3 ambient_light = { 0.1f, 0.1f, 0.1f };
    glm::vec3 skybox_tint = { 1.0f, 1.0f, 1.0f };

    // Atmopshere
    CloudSettings cloud_settings = {};

    Camera camera = {};
};
