#pragma once
#include <vector>
#include "entity.h"
#include "camera.h"
#include "water.h"

struct Scene
{
    std::vector<Entity> entities = {};
    std::vector<Water> waters = {};
    Camera camera = {};
};
