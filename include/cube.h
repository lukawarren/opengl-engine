#pragma once
#include <vector>
#include <glm/glm.hpp>

const glm::vec3 start = { -0.5f, -0.5f, -0.5f };
const glm::vec3 end   = {  0.5f,  0.5f,  0.5f };

const std::vector<float> cube_vertices =
{
    start.x, end.y, start.z, start.x, start.y, start.z,
    end.x, start.y, start.z, end.x, end.y, start.z,
    start.x, end.y, end.z, start.x, start.y, end.z,
    end.x, start.y, end.z, end.x, end.y, end.z,
    end.x, end.y, start.z, end.x, start.y, start.z,
    end.x, start.y, end.z, end.x, end.y, end.z,
    start.x, end.y, start.z, start.x, start.y, start.z,
    start.x, start.y, end.z, start.x, end.y, end.z,
    start.x, end.y, end.z, start.x, end.y, start.z,
    end.x, end.y, start.z, end.x, end.y, end.z,
    start.x, start.y, end.z,start.x, start.y, start.z,
    end.x, start.y, start.z,end.x, start.y, end.z
};

const std::vector<unsigned int> cube_indices =
{
    3,1,0,2,1,3,
    4,5,7,7,5,6,
    11,9,8,10,9,11,
    12,13,15,15,13,14,
    19,17,16,18,17,19,
    20,21,23,23,21,22
};

const std::vector<float> cube_texture_coords =
{
    0,0,0,1,1,1,
    1,0,0,0,0,1,
    1,1,1,0,0,0,
    0,1,1,1,1,0,
    0,0,0,1,1,1,
    1,0,0,0,0,1,
    1,1,1,0,0,0,
    0,1,1,1,1,0
};

const std::vector<float> cube_normals =
{
    0, 0, -1, 0, 0, -1,
    0, 0, -1, 0, 0, -1,
    0, 0, 1, 0, 0, 1,
    0, 0, 1, 0, 0, 1,
    1, 0, 0, 1, 0, 0,
    1, 0, 0, 1, 0, 0,
    -1, 0, 0, -1, 0, 0,
    -1, 0, 0, -1, 0, 0,
    0, 1, 0, 0, 1, 0,
    0, 1, 0, 0, 1, 0,
    0, -1, 0, 0, -1, 0,
    0, -1, 0, 0, -1, 0,
};
