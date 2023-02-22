#pragma once
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Texture
{
public:
    Texture(const std::string& filename);

    // For use with FBOs, etc.
    Texture(
        const unsigned int width,
        const unsigned int height,
        const unsigned int internal_format = GL_RGB,
        const unsigned int format = GL_RGB,
        const unsigned int type = GL_UNSIGNED_BYTE);

    Texture(const Texture&) = delete;
    ~Texture();

    void clamp(const glm::vec4& colour) const;
    void set_as_texture_atlas(const int max_mipmap_level) const;

    void bind(const unsigned int unit = 0) const;
    void unbind() const;

    // OpenGL state
    unsigned int texture_id;
};