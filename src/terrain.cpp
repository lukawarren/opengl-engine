#include "terrain.h"
#include "stb_image.h"
#include "resources.h"
#include <numeric>
#include <array>

constexpr size_t vertices_per_side = 256;
constexpr size_t n_vertices = vertices_per_side * vertices_per_side;
constexpr float horizontal_scale = 100.0f;
constexpr float vertical_scale = 0.4f;

Terrain::Terrain(const std::string& diffuse_texture, const std::string& height_map)
{
    // Load heightmap
    int width, height, channels;
    unsigned char* data = stbi_load(("../res/assets/" + height_map).c_str(), &width, &height, &channels, 0);
    if (!data) throw std::runtime_error("failed to load terrain height map " + height_map);

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
            const float x = ((float)j / ((float)vertices_per_side - 1) - 0.5f) * horizontal_scale;
            const float z = ((float)i / ((float)vertices_per_side - 1) - 0.5f) * horizontal_scale;

            // Sample heightmap
            const auto get_height = [&](size_t grid_x, size_t grid_z)
            {
                const int image_x = ((float)grid_x / ((float)vertices_per_side)) * (float)width;
                const int image_y = ((float)grid_z / ((float)vertices_per_side)) * (float)height;
                if (image_x < 0 || image_y < 0 || image_x >= width || image_y >= height) return 0.0f;

                const float image_sample = data[image_y * width + image_x];
                return image_sample * vertical_scale;
            };

            const std::array<float, 4> heights =
            {
                get_height(i - 1, j - 0),
                get_height(i + 1, j - 0),
                get_height(i + 0, j - 1),
                get_height(i + 0, j + 1),
            };

            // Average surrounding height
            const float y = std::accumulate(heights.begin(), heights.end(), 0.0f) / heights.size();

            vertices.emplace_back(x);
            vertices.emplace_back(y);
            vertices.emplace_back(z);

            // Work out normals from surrounding heightmap values
            const float height_l = heights[0];
            const float height_r = heights[1];
            const float height_d = heights[2];
            const float height_u = heights[3];
            glm::vec3 normal = glm::vec3(height_l - height_r, 2.0f, height_d - height_u);
            normal = glm::normalize(normal);

            normals.emplace_back(normal.x);
            normals.emplace_back(normal.y);
            normals.emplace_back(normal.z);

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
    texture = get_texture(diffuse_texture);
    stbi_image_free(data);
}
