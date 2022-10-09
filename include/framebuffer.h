#pragma once
#include "texture.h"
#include <optional>

class Framebuffer
{
public:
    Framebuffer(const unsigned int width, const unsigned int height, const bool depth_map = false);
    Framebuffer(const Framebuffer&) = delete;
    ~Framebuffer();

    void bind() const;
    void unbind() const;

    unsigned int width;
    unsigned int height;

    Texture colour_texture;
    std::optional<Texture> depth_map;

private:
    unsigned int fbo;
    std::optional<unsigned int> rbo;
};
