#pragma once
#include <vector>
#include <cstddef>
#include <optional>

class Mesh
{
public:
    Mesh(
        const std::vector<float>& vertices,
        const std::vector<unsigned int>& indices,
        const std::vector<float>& texture_coords,
        const std::optional<std::vector<float>>& normals = {}
    );
    Mesh(const Mesh&) = delete;
    ~Mesh();

    static Mesh* quad();

    void bind() const;
    void unbind() const;
    void draw() const;

private:

    void make_vao(
        const unsigned int attribute,
        const unsigned int format,
        const unsigned int dimensions,
        const std::vector<float>& data
    );

    // OpenGL state
    unsigned int vao;
    unsigned int ebo;
    std::vector<unsigned int> vbos;

    // Mesh info
    size_t indices;
};