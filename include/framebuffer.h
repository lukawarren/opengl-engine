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
        const DepthSettings depth_settings = DepthSettings::NO_DEPTH,
        const bool is_g_buffer = false
    );
    Framebuffer(const Framebuffer&) = delete;
    ~Framebuffer();

    void bind() const;
    void unbind(unsigned int previous_width, unsigned int previous_height) const;

    unsigned int width;
    unsigned int height;

    std::optional<Texture> colour_texture;
    std::optional<Texture> normal_texture;
    std::optional<Texture> position_texture;
    std::optional<Texture> depth_map;

private:
    unsigned int fbo;
    std::optional<unsigned int> rbo;
};
