#pragma once
#include "transform.h"

class Water
{
public:
    Water(Transform _transform = {
        {},
        { 90.0f, 0.0f, 0.0f },
        { 10.0f, 10.0f, 10.0f }
    }) : transform(_transform) {}
    Transform transform;
};