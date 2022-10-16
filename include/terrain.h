#pragma once
#include "mesh.h"
#include "transform.h"
#include "texture.h"
#include <memory>
#include <string>

class Terrain
{
public:
    Terrain(const std::string& diffuse_texture, const std::string& height_map);

    // Shared pointer due to copy constructor woes (as elsewhere - TODO: fix)
    std::shared_ptr<Mesh> mesh;
    Transform transform = {};
    Texture* texture;
};