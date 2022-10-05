#pragma once
#include "mesh.h"
#include <vector>

class Entity
{
public:
    Entity(const std::string& filename);
    std::vector<Mesh*> meshes;
};