#pragma once
#include "resources.h"
#include "transform.h"
#include <vector>
#include <string>

class Entity
{
public:
    Entity(const std::string& filename) :
        textured_meshes(load_assimp_scene(filename)) {}

    std::vector<TexturedMesh> textured_meshes;
    Transform transform = {};
};