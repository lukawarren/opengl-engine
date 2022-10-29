#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>

Texture::Texture(const std::string& filename)
{
    // Load from disk
    int width, height, channels;
    unsigned char* data = stbi_load(("../res/assets/" + filename).c_str(), &width, &height, &channels, 0);
    if (!data) throw std::runtime_error("failed to load texture " + filename);

    // Create and bind texture
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    // Upload data
    const auto format = (channels == 3 ? GL_RGB : GL_RGBA);
    const auto formatInternal = (channels == 3 ? GL_RGB8 : GL_RGBA8);
    glTexImage2D(GL_TEXTURE_2D, 0, formatInternal, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    // Mipmaps
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Texture wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // Do anisotropic filtering (if we can)
    if (GLAD_GL_EXT_texture_filter_anisotropic)
    {
        float max_anisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, std::min(max_anisotropy, 16.0f));
    }
    else std::cerr << "anisotropic filtering not supported" << std::endl;

    // Unbind and free image from normal memory
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);
}

Texture::Texture(
    const unsigned int width,
    const unsigned int height,
    const unsigned int internal_format,
    const unsigned int format,
    const unsigned int type)
{
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, format, type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // For water FBOs
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // For water FBOs
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::clamp(const glm::vec4& colour) const
{
    const float border_colour[] = { colour.r, colour.g, colour.b, colour.a };
    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_colour);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(const unsigned int unit) const
{
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture_id);
}

void Texture::unbind() const
{
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture()
{
    glDeleteTextures(1, &texture_id);
}
