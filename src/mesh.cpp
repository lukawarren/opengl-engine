#include "mesh.h"
#include "cube.h"
#include "quad.h"
#include <glad/glad.h>
#include <stdexcept>

Mesh::Mesh(
    const std::vector<float>& vertices,
    const std::vector<unsigned int>& indices,
    const std::vector<float>& texture_coords,
    const std::optional<std::vector<float>>& normals,
    const std::optional<std::vector<float>>& tangents
)
{
    // Create and bind VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Create, bind and upload indices' element buffer object (EBO)
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), &indices[0], GL_STATIC_DRAW);

    // Make VAOs
    make_vao(0, GL_FLOAT, 3, vertices);
    make_vao(1, GL_FLOAT, 2, texture_coords);
    if (normals.has_value()) make_vao(2, GL_FLOAT, 3, normals.value());
    if (tangents.has_value()) make_vao(3, GL_FLOAT, 3, tangents.value());

    // Unbind VAO but *not* EBO (as this is bound by the VAO for us)
    glBindVertexArray(0);
    this->indices = indices.size();
}

Mesh* Mesh::quad()
{
    return new Mesh(quad_vertices, quad_indices, quad_texture_coords);
}

Mesh* Mesh::cube()
{
    return new Mesh(cube_vertices, cube_indices, cube_texture_coords);
}

void Mesh::make_vao(
    const unsigned int attribute,
    const unsigned int format,
    const unsigned int dimensions,
    const std::vector<float>& data
)
{
    // Make and fill VBO
    unsigned int vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(data[0]), &data[0], GL_STATIC_DRAW);

    // Inform OpenGL of the nature of the data - GL_FALSE disables normalisation of data
    glVertexAttribPointer(attribute, dimensions, format, GL_FALSE, dimensions * sizeof(data[0]), (void*)0);
    glEnableVertexAttribArray(attribute);

    // Unbind then keep track of VBO for future clean-up
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    vbos.push_back(vbo);
}

void Mesh::bind() const
{
    glBindVertexArray(vao);
}

void Mesh::unbind() const
{
    glBindVertexArray(0);
}

void Mesh::draw() const
{
    glDrawElements(GL_TRIANGLES, indices, GL_UNSIGNED_INT, 0);
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1, &vao);
    for (const auto vbo: vbos)
        glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
}
