#pragma once
#include <array>
#include <vector>

const std::array<std::vector<float>, 6> face_vertices =
{{
    // Top
    {
         0.5f, 0.5f,  0.5f,
         0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f,  0.5f
    },

    // Bottom
    {
         0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f
    },

    // Left
    {
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f
    },

    // Right
    {
        0.5f,  0.5f,  0.5f,
        0.5f, -0.5f,  0.5f,
        0.5f, -0.5f, -0.5f,
        0.5f,  0.5f, -0.5f
    },

    // Front
    {
         0.5f,  0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f
    },

    // Back
    {
         0.5f,  0.5f, 0.5f,
        -0.5f,  0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f,
         0.5f, -0.5f, 0.5f,
    }
}};

const std::array<unsigned int, 6> face_indices =
{
    0, 1, 3,
    1, 2, 3
};

const std::array<float, 8> face_texture_coords =
{
    1.0f, 1.0f,
    1.0f, 0.0f,
    0.0f, 0.0f,
    0.0f, 1.0f
};

const std::array<glm::vec3, 6> face_normals =
{{
    {  0,  1,  0 }, // Top
    {  0, -1,  0 }, // Bottom
    { -1,  0,  0 }, // Left
    {  1,  0,  0 }, // Right
    {  0,  0,  1 }, // Front
    {  0,  0, -1 }  // Back
}};
