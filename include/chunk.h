#pragma once
#include <memory>
#include "transform.h"
#include "texture.h"
#include "mesh.h"

enum class Block
{
    Air = 0,
    Grass,
    Dirt,
    Stone,
    Sand
};

class Chunk
{
public:
    Chunk(const glm::ivec3 position);
    ~Chunk();

    // State common to all chunks
    static Texture* texture;
    static constexpr int size = 32;

    // State for this chunk - shared_ptr to solve memory woes (as elsewhere)
    Transform transform = {};
    std::shared_ptr<Mesh> mesh;
    Block blocks[size][size][size] = {};

private:
    void generate_blocks(const glm::ivec3 position);
    void generate_mesh();

    std::array<float, 8> get_texture_coords_for_block(const Block block) const;
};