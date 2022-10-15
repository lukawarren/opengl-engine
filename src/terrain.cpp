#include "terrain.h"

/*
    Terrain mesh generation based off
    series by "ThinMatrix"
*/

constexpr size_t vertices_per_side = 1000;
constexpr size_t n_vertices = vertices_per_side * vertices_per_side;
constexpr float scale = 100.0f;

Terrain::Terrain()
{
    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> texture_coords;
    std::vector<unsigned int> indices;

    // Reserve space
    vertices.reserve(n_vertices * 3);
    normals.reserve(n_vertices * 3);
    texture_coords.reserve(n_vertices * 2);
    indices.reserve(6 * (vertices_per_side-1) * (vertices_per_side-1));

    // Create mesh data
    for (size_t i = 0; i < vertices_per_side; ++i)
    {
        for (size_t j = 0; j < vertices_per_side; ++j)
        {
            // Have origin of terrain be in its middle
            const auto x = ((float)j / ((float)vertices_per_side - 1) - 0.5f) * scale;
            const auto z = ((float)i / ((float)vertices_per_side - 1) - 0.5f) * scale;

            vertices.emplace_back(x);
            vertices.emplace_back(0);
            vertices.emplace_back(z);

            normals.emplace_back(0);
            normals.emplace_back(1);
            normals.emplace_back(0);

            texture_coords.emplace_back((float)j / ((float)vertices_per_side-1));
            texture_coords.emplace_back((float)i / ((float)vertices_per_side-1));
        }
    }

    // Create indices
    for (size_t z = 0; z < vertices_per_side-1; ++z)
    {
        for (size_t x = 0; x < vertices_per_side-1; ++x)
        {
            const auto top_left = (z * vertices_per_side) + x;
            const auto top_right = top_left + 1;
            const auto bottom_left = ((z+1) * vertices_per_side) + x;
            const auto bottom_right = bottom_left + 1;

            indices.emplace_back(top_left);
            indices.emplace_back(bottom_left);
            indices.emplace_back(top_right);
            indices.emplace_back(top_right);
            indices.emplace_back(bottom_left);
            indices.emplace_back(bottom_right);
        }
    }

    // Load to GPU
    mesh = std::make_shared<Mesh>(vertices, indices, texture_coords, normals);
}