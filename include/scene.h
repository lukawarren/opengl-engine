#pragma once
#include <vector>
#include "entity.h"
#include "camera.h"
#include "water.h"
#include "terrain.h"

struct Scene
{
    std::vector<Entity> entities = {};
    std::vector<Water> waters = {};
    std::vector<Terrain> terrains;
    Camera camera = {};
};
