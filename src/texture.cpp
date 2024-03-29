#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

Texture::Texture(const std::string& filename, const bool use_nearest_filtering)
{
    // Load from disk
    int width, height, channels;
    unsigned char* data = stbi_load(("../res/assets/" + filename).c_str(), &width, &height, &channels, 0);
    if (!data) throw std::runtime_error("failed to load texture " + filename);

    // Create and bind texture
    glGenTextures(1, &texture_id);
    glBindTexture(texture_type, texture_id);

    // Upload data
    const auto format = (channels == 3 ? GL_RGB : GL_RGBA);
    const auto formatInternal = (channels == 3 ? GL_RGB8 : GL_RGBA8);
    glTexImage2D(texture_type, 0, formatInternal, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Mipmaps
    glGenerateMipmap(texture_type);
    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, use_nearest_filtering ? GL_NEAREST_MIPMAP_LINEAR : GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, use_nearest_filtering ? GL_NEAREST : GL_LINEAR);

    // Texture wrapping
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Do anisotropic filtering (if we can)
    if (!use_nearest_filtering)
    {
        if (GLAD_GL_EXT_texture_filter_anisotropic)
        {
            float max_anisotropy;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
            glTexParameterf(texture_type, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(max_anisotropy, 16.0f));
        }
        else std::cerr << "anisotropic filtering not supported" << std::endl;
    }

    // Unbind and free image from normal memory
    glBindTexture(texture_type, 0);
    stbi_image_free(data);
}

Texture::Texture(const std::array<std::string, 6> faces)
{
    glGenTextures(1, &texture_id);
    glBindTexture(texture_type, texture_id);

    // Load from disk
    int width, height, channels;
    for (unsigned int i = 0; i < faces.size(); ++i)
    {
        unsigned char* data = stbi_load(("../res/assets/" + faces[i]).c_str(), &width, &height, &channels, 0);
        if (!data) throw std::runtime_error("failed to load texture " + faces[i]);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    // Sampling options - no need for mipmaps as skyboxes, etc. aren't small
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(texture_type, 0);
}

Texture::Texture(
    const unsigned int width,
    const unsigned int height,
    const unsigned int internal_format,
    const unsigned int format,
    const unsigned int type,
    const bool use_nearest_filtering,
    const char* data,
    const unsigned int depth)
{
    if (depth)
        texture_type = GL_TEXTURE_3D;

    glGenTextures(1, &texture_id);
    glBindTexture(texture_type, texture_id);

    if (depth) glTexImage3D(texture_type, 0, internal_format, width, height, depth, 0, format, type, data);
    else glTexImage2D(texture_type, 0, internal_format, width, height, 0, format, type, data);

    glTexParameteri(texture_type, GL_TEXTURE_MIN_FILTER, use_nearest_filtering ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(texture_type, GL_TEXTURE_MAG_FILTER, use_nearest_filtering ? GL_NEAREST : GL_LINEAR);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(texture_type, 0);
}

void Texture::clamp(const glm::vec4& colour, const bool to_border) const
{
    const float border_colour[] = { colour.r, colour.g, colour.b, colour.a };
    glBindTexture(texture_type, texture_id);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_S, to_border ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE);
    glTexParameteri(texture_type, GL_TEXTURE_WRAP_T, to_border ? GL_CLAMP_TO_BORDER : GL_CLAMP_TO_EDGE);
    glTexParameterfv(texture_type, GL_TEXTURE_BORDER_COLOR, border_colour);
    glBindTexture(texture_type, 0);
}

void Texture::set_as_texture_atlas(const int max_mipmap_level) const
{
    // Limit mip-mapping so sub-textures don't go smaller than 1x1
    glBindTexture(texture_type, texture_id);
    glTexParameteri(texture_type, GL_TEXTURE_MAX_LEVEL, max_mipmap_level);
    glBindTexture(texture_type, 0);
}

void Texture::bind(const unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(texture_type, texture_id);
}

void Texture::bind_image(const unsigned int internal_format, const unsigned int access) const
{
    glBindImageTexture(0, texture_id, 0, GL_FALSE, 0, access, internal_format);
}

void Texture::unbind() const
{
    glBindTexture(texture_type, 0);
}

Texture::~Texture()
{
    glDeleteTextures(1, &texture_id);
}
