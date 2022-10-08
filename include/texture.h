#pragma once
#include <string>

class Texture
{
public:
    Texture(const std::string& filename);

    // For use with FBOs, etc.
    Texture(const unsigned int width, const unsigned int height);

    Texture(const Texture&) = delete;
    ~Texture();

    void bind() const;
    void unbind() const;

    // OpenGL state
    unsigned int texture_id;
};