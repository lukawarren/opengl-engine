#pragma once
#include "mesh.h"
#include <vector>
#include <string>
#include <memory>

class Entity
{
public:
    Entity(const std::string& filename);
    ~Entity();

    // TODO: allow entities to share the same mesh (and textures too).
    // NOTE: right now a shared_ptr is used to solve copy constructor woes.
    std::vector<std::shared_ptr<Mesh>> meshes;
};