#include "chunk.h"
#include "entity.h"
#include "chunk_faces.h"
#include <glm/gtc/noise.hpp>

Texture* Chunk::texture = nullptr;

Chunk::Chunk(const glm::ivec3 position)
{
    if (!texture) texture = get_texture("grass.png");

    generate_blocks(position);
    generate_mesh();

    // Convert chunks-space position to world-space
    transform.position = position * glm::ivec3 { size, size, size };
}

void Chunk::generate_blocks(const glm::ivec3 position)
{
    const auto get_height = [&](const int x, const int z)
    {
        const float height_scale = 3.0f;
        const float noise_scale = 0.1f;

        const auto val = glm::simplex(glm::vec2 {
            position.x * size + x,
            position.x * size + z
        } * noise_scale);

        return (val + 1.0f) / 2.0f * height_scale;
    };

    for (int x = 0; x < size; ++x)
    {
        for (int z = 0; z < size; ++z)
        {
            int height = (int)get_height(x, z);
            if (height <= 0) height = 1;

            for (int y = 0; y < height; ++y)
                blocks[x][y][z] = Block::Grass;
        }
    }
}

void Chunk::generate_mesh()
{
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

                        // Corresponding normals
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
}

Chunk::~Chunk()
{
}