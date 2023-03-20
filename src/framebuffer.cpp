#include <glad/glad.h>
#include <stdexcept>
#include "framebuffer.h"

Framebuffer::Framebuffer(const unsigned int width, const unsigned int height, const DepthSettings depth_settings, const bool is_g_buffer)
{
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Add colour attachment (if need be)
    if (depth_settings != DepthSettings::ONLY_DEPTH)
    {
        this->colour_texture.emplace(
            width,
            height,
            GL_RGBA16F, // HDR
            GL_RGBA,    // HDR
            GL_FLOAT    // HDR
        );
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D,
            colour_texture->texture_id,
            0
        );
    }
    else
    {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }

    // If G-buffer, add normal and position textures
    if (is_g_buffer)
    {
        this->normal_texture.emplace(width, height, GL_RGB32F, GL_RGB, GL_FLOAT);
        this->position_texture.emplace(width, height, GL_RGB32F, GL_RGB, GL_FLOAT);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_texture->texture_id, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, position_texture->texture_id, 0);
        unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, attachments);
    }

    if (depth_settings != DepthSettings::NO_DEPTH)
    {
        // Create depth buffer using *nearest* filtering!
        this->depth_map.emplace(
            width,
            height,
            GL_DEPTH_COMPONENT24,
            GL_DEPTH_STENCIL,
            GL_UNSIGNED_INT_24_8,
            true
        );
        this->depth_map->clamp(glm::vec4(1.0f));
        glFramebufferTexture2D(
            GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            GL_TEXTURE_2D,
            this->depth_map->texture_id,
            0
        );
    }
    else if (depth_settings == DepthSettings::NO_DEPTH)
    {
        // Create renderbuffer for depth and stencil buffers, as we don't
        // mean to read from them
        unsigned int _rbo;
        glGenRenderbuffers(1, &_rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, _rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glFramebufferRenderbuffer(
            GL_FRAMEBUFFER,
            GL_DEPTH_STENCIL_ATTACHMENT,
            GL_RENDERBUFFER,
            _rbo
        );
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        rbo.emplace(_rbo);
    }

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("incomplete framebuffer - " +
            std::to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    this->width = width;
    this->height = height;
}

void Framebuffer::bind() const
{
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glViewport(0,0, width, height);
}

void Framebuffer::unbind(unsigned int previous_width, unsigned int previous_height) const
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0,0, previous_width, previous_height);
}

Framebuffer::~Framebuffer()
{
    if (rbo.has_value())
        glDeleteRenderbuffers(1, &rbo.value());

    glDeleteFramebuffers(1, &fbo);
}
