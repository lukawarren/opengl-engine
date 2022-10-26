#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "framebuffer.h"

constexpr unsigned int shadowmap_size = 2048;

struct DirectionalLight
{
    glm::vec3 position = {};
    glm::vec3 colour = {};
    std::shared_ptr<Framebuffer> shadow_buffer;

    DirectionalLight(const glm::vec3& position = {}, const glm::vec3& colour = {})
    {
        this->position = position;
        this->colour = colour;
        shadow_buffer = std::make_shared<Framebuffer>(
            shadowmap_size,
            shadowmap_size,
            Framebuffer::DepthSettings::ONLY_DEPTH
        );
    }

    glm::mat4 get_light_projection_matrix() const
    {
        const float near = 0.01f;
        const float far = 100.0f;
        const float size = 10.0f;

        glm::mat4 lightProjection = glm::ortho(-size, size, -size, size, near, far);
        glm::mat4 lightView = glm::lookAt
        (
            position,                     // position
            glm::vec3(0.0f, 0.0f,  0.0f), // centre
            glm::vec3(0.0f, 1.0f,  0.0f)  // up direction
        );

        return lightProjection * lightView;
    }

};
