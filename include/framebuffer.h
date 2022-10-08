#pragma once
#include "texture.h"

class Framebuffer
{
public:
    Framebuffer(const unsigned int width, const unsigned int height);
    Framebuffer(const Framebuffer&) = delete;
    ~Framebuffer();

    void bind() const;
    void unbind() const;

    unsigned int width;
    unsigned int height;
    Texture colour_texture;

private:
    unsigned int fbo;
    unsigned int rbo;
};
