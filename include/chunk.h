#pragma once
#include <memory>
#include "transform.h"
#include "texture.h"
#include "mesh.h"

enum class Block
{
    Air = 0,
    Grass
};

class Chunk
{
public:
    Chunk(const glm::ivec3 position);
    ~Chunk();

    // State common to all chunks
    static Texture* texture;

    // State for this chunk - shared_ptr to solve memory woes (as elsewhere)
    Transform transform = {};
    std::shared_ptr<Mesh> mesh;

    static constexpr int size = 16;
};