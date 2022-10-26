#pragma once
#include "texture.h"
#include <optional>

class Framebuffer
{
public:

    enum class DepthSettings
    {
        NO_DEPTH,
        ENABLE_DEPTH,
        ONLY_DEPTH
    };

    Framebuffer(
        const unsigned int width,
        const unsigned int height,
        const DepthSettings depth_settings = DepthSettings::NO_DEPTH
    );
    Framebuffer(const Framebuffer&) = delete;
    ~Framebuffer();

    void bind() const;
    void unbind() const;

    unsigned int width;
    unsigned int height;

    std::optional<Texture> colour_texture;
    std::optional<Texture> depth_map;

private:
    unsigned int fbo;
    std::optional<unsigned int> rbo;
};
