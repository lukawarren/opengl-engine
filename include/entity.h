#pragma once
#include "mesh.h"
#include "texture.h"
#include "transform.h"
#include <vector>
#include <string>
#include <memory>

class Entity
{
public:
    Entity(const std::string& filename, const std::string& texture);
    ~Entity();

    // TODO: allow entities to share the same mesh (and textures too,
    //       with each one being different).
    // NOTE: right now pointers are used to solve copy constructor woes.
    std::vector<std::shared_ptr<Mesh>> meshes;
    std::shared_ptr<Texture> texture;
    Transform transform = {};
};