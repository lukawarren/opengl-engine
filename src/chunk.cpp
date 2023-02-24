#include "chunk.h"
#include "entity.h"
#include "chunk_faces.h"
#include <glm/gtc/noise.hpp>
#include <stdexcept>

Texture* Chunk::texture = nullptr;
const glm::vec2 texture_size = { 256.0f, 256.0f };
const glm::vec2 subtexture_size = { 16.0f, 16.0f };

Chunk::Chunk(const glm::ivec3 position)
{
    if (!texture)
    {
        texture = get_texture("blocks.png", true);
        texture->set_as_texture_atlas(1);
    }

    generate_blocks(position);
    generate_mesh();

    // Convert chunks-space position to world-space
    transform.position = position * glm::ivec3 { size, size, size };
}

void Chunk::generate_blocks(const glm::ivec3 position)
{
    const auto sample_noise = [](glm::vec2 pos, float scale)
    {
        const auto val = glm::simplex(pos * scale);
        return (val + 1.0f) / 2.0f;
    };

    const auto get_height = [&](const int x, const int z)
    {
        const glm::vec2 pos = { position.x * size + x, position.z * size + z };
        return sample_noise(pos, 1 / 64.0f) * 20.0f;
    };

    // Base terrain layer
    std::vector<int> highest_points;
    highest_points.reserve(size*size);
    for (int x = 0; x < size; ++x)
    {
        for (int z = 0; z < size; ++z)
        {
            int height = (int)get_height(x, z);
            if (height <= 0) height = 1;
            if (height >= max_height) height = max_height - 1;

            highest_points.emplace_back(height);

            for (int y = 0; y < height; ++y)
            {
                if (y < 5) blocks[x][y][z] = Block::Sand;
                else if (y == height-1) blocks[x][y][z] = Block::Grass;
                else blocks[x][y][z] = Block::Dirt;
            }
        }
    }

    // Trees
    for (int i = 0; i < 5; ++i)
    {
        size_t index = rand() % highest_points.size();
        int height = highest_points[index];
        int x = index / size;
        int z = index % size;

        for (int y = 0; y < 6; ++y)
        {
            blocks[x][height + y][z] = Block::Wood;

            const auto place = [&](int x, int y, int z, Block b)
            {
                if (x >= 0 && y >= 0 && x < size && y < size)
                    blocks[x][y][z] = b;
            };

            if (y >= 4)
            {
                place(x - 1, y + height, z - 1, Block::Leaves);
                place(x + 0, y + height, z - 1, Block::Leaves);
                place(x + 1, y + height, z - 1, Block::Leaves);

                place(x - 1, y + height, z + 0, Block::Leaves);
                place(x + 0, y + height, z + 0, Block::Leaves);
                place(x + 1, y + height, z + 0, Block::Leaves);

                place(x - 1, y + height, z + 1, Block::Leaves);
                place(x + 0, y + height, z + 1, Block::Leaves);
                place(x + 1, y + height, z + 1, Block::Leaves);
            }
        }
    }
}

void Chunk::generate_mesh()
{
    const auto is_solid_block = [&](int x, int y, int z)
    {
        // TODO: check against adjacent chunks
        if (x < 0 || y < 0 || z < 0) return false;
        if (x >= size || y >= max_height || z >= size) return false;
        return blocks[x][y][z] != Block::Air;
    };

    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    std::vector<float> texture_coordinates;
    std::vector<float> normals;
    unsigned int faces = 0;

    for (int x = 0; x < size; ++x)
    {
        for (int y = 0; y < max_height; ++y)
        {
            for (int z = 0; z < size; ++z)
            {
                const auto add_face = [&](int n, Block block)
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

                    // Tex coords don't change, but depend on the blcok
                    const auto uvs = get_texture_coords_for_block(block, n == 0);
                    texture_coordinates.insert(texture_coordinates.end(), uvs.begin(), uvs.end());

                    // For indices, offset past existing geometry
                    for (const auto index : face_indices)
                        indices.emplace_back(index + faces * face.size() / 3);
                    ++faces;
                };

                auto block = blocks[x][y][z];
                if (block == Block::Air) continue;
                if (!is_solid_block(x, y + 1, z)) add_face(0, block);
                if (!is_solid_block(x, y - 1, z)) add_face(1, block);
                if (!is_solid_block(x - 1, y, z)) add_face(2, block);
                if (!is_solid_block(x + 1, y, z)) add_face(3, block);
                if (!is_solid_block(x, y, z - 1)) add_face(4, block);
                if (!is_solid_block(x, y, z + 1)) add_face(5, block);
            }
        }
    }

    // Upload to GPU
    mesh = std::make_shared<Mesh>(vertices, indices, texture_coordinates, normals);
}

std::array<float, 8> Chunk::get_texture_coords_for_block(const Block block, const bool is_top_face) const
{
    const auto get_nth_item_in_atlas = [&](const Block block)
    {
        switch (block)
        {
            case Block::Grass:  return is_top_face ?  0 : 1;
            case Block::Dirt:   return 2;
            case Block::Stone:  return 3;
            case Block::Sand:   return 7;
            case Block::Wood:   return is_top_face ? 5 : 4;
            case Block::Leaves: return 6;
            default:
                throw std::runtime_error("block with unknown texture atlas position");
        }
    };

    // Find block
    const glm::vec2 block_offset = subtexture_size / texture_size;
    const glm::vec2 offset_bottom = block_offset * glm::vec2{ get_nth_item_in_atlas(block), 0.0f };
    const glm::vec2 offset_top = offset_bottom + block_offset;

    return {
        offset_bottom.x,    offset_bottom.y,
        offset_bottom.x,    offset_top.y,
        offset_top.x,       offset_top.y,
        offset_top.x,       offset_bottom.y
    };
}

Chunk::~Chunk() {}
