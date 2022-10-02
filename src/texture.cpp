#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glad/glad.h>
#include <iostream>

Texture::Texture(const std::string& filename)
{
    // Load from disk
    int width, height, channels;
    unsigned char* data = stbi_load(("../res/textures/" + filename).c_str(), &width, &height, &channels, 0);
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

void Texture::bind() const
{
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
