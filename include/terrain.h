#pragma once
#include "mesh.h"
#include "transform.h"
#include <memory>

class Terrain
{
public:
    Terrain();

    // Shared pointer due to copy constructor woes (as elsewhere - TODO: fix)
    std::shared_ptr<Mesh> mesh;
    Transform transform = {};
};