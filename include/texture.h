#pragma once
#include <string>
#include "resource.h"

class Texture : GLResource
{
public:
    Texture(const std::string& filename);
    Texture(const Texture&) = delete;
    ~Texture();

    void bind() const;
    void unbind() const;

private:
    // OpenGL state
    unsigned int texture_id;
};