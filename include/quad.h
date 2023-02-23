#pragma once
#include <vector>

const std::vector<float> quad_vertices =
{
     1.0f,  1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
    -1.0f,  1.0f, 0.0f
};

const std::vector<unsigned int> quad_indices =
{
    0, 1, 3,
    1, 2, 3
};

const std::vector<float> quad_texture_coords =
{
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f
};
