#include "chunk.h"
#include "entity.h"
#include <array>
#include <iostream>
int remove_me;

Texture* Chunk::texture = nullptr;

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

Chunk::Chunk(const glm::ivec3 position)
{
    if (!texture) texture = get_texture("missing_texture.png");

    // Generate terrain
    Block blocks[size][size][size] = {};
    for (int x = 0; x < size; ++x)
    for (int y = 0; y < size; ++y)
    for (int z = 0; z < size; ++z)
    blocks[x][y][z] = (Block)(rand() % 2);

    const auto is_solid_block = [&](int x, int y, int z)
    {
        // TODO: check against adjacent chunks
        if (x < 0 || y < 0 || z < 0) return false;
        if (x >= size || y >= size || z >= size) return false;
        return blocks[x][y][z] != Block::Air;
    };

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> texture_coordinates;
    std::vector<float> normals;
    unsigned int faces = 0;

    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < size; ++y)
        {
            for (int z = 0; z < size; ++z)
            {
                const auto add_face = [&](int n)
                {
                    // For vertices, add the block position to each vertex (every 3 elements)
                    const auto& face = face_vertices[n];
                    for (unsigned int i = 0; i < face.size() / 3; ++i)
                    {
                        vertices.emplace_back(face[i * 3 + 0] + x);
                        vertices.emplace_back(face[i * 3 + 1] + y);
                        vertices.emplace_back(face[i * 3 + 2] + z);
                        normals.emplace_back(face_normals[n].x);
                        normals.emplace_back(face_normals[n].y);
                        normals.emplace_back(face_normals[n].z);
                    }

                    // Tex coords don't change
                    texture_coordinates.insert(
                        texture_coordinates.end(),
                        face_texture_coords.begin(),
                        face_texture_coords.end()
                    );

                    // For indices, offset past existing geometry
                    for (const auto index : face_indices)
                        indices.emplace_back(index + faces * face.size() / 3);
                    ++faces;
                };

                if (blocks[x][y][z] == Block::Air) continue;
                if (!is_solid_block(x, y + 1, z)) add_face(0);
                if (!is_solid_block(x, y - 1, z)) add_face(1);
                if (!is_solid_block(x - 1, y, z)) add_face(2);
                if (!is_solid_block(x + 1, y, z)) add_face(3);
                if (!is_solid_block(x, y, z - 1)) add_face(4);
                if (!is_solid_block(x, y, z + 1)) add_face(5);
            }
        }
    }

    // Upload to GPU
    mesh = std::make_shared<Mesh>(vertices, indices, texture_coordinates, normals);

    // Convert chunks-space position to world-space
    transform.position = position * glm::ivec3 { size, size, size };
}

Chunk::~Chunk()
{
}