#pragma once
#include <string>
#include <glad/glad.h>

class Texture
{
public:
    Texture(const std::string& filename);

    // For use with FBOs, etc.
    Texture(
        const unsigned int width,
        const unsigned int height,
        const unsigned int internal_format = GL_RGB,
        const unsigned int format = GL_UNSIGNED_BYTE);

    Texture(const Texture&) = delete;
    ~Texture();

    void bind(const unsigned int unit = 0) const;
    void unbind() const;

    // OpenGL state
    unsigned int texture_id;
};