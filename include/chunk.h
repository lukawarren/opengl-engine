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
    Sand,
    Wood,
    Leaves
};

struct BlockPosition
{
    int chunk;
    int x;
    int y;
    int z;
};

class Chunk
{
public:
    Chunk(const glm::ivec3 position);
    ~Chunk();

    // State common to all chunks
    static Texture* texture;
    static constexpr int size = 32;
    static constexpr int max_height = 256;

    // State for this chunk - shared_ptr to solve memory woes (as elsewhere)
    Transform transform = {};
    std::shared_ptr<Mesh> mesh;
    Block blocks[size][max_height][size] = {};

    void rebuild_mesh();

private:
    void generate_blocks(const glm::ivec3 position);
    void generate_mesh();

    std::array<float, 8> get_texture_coords_for_block(const Block block, const bool is_top_face) const;
};