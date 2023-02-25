#pragma once
#include <memory>
#include <glm/glm.hpp>
#include "framebuffer.h"
#include "camera.h"

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
            SHADOWMAP_RESOLUTION,
            SHADOWMAP_RESOLUTION,
            Framebuffer::DepthSettings::ONLY_DEPTH
        );
    }

    glm::mat4 get_light_projection_matrix(const Camera& camera, const float width, const float height) const
    {
        // Bounds of what we want to be shadowed
        const std::array<glm::vec4, 8> bounds_shadowed = camera.get_frustum_corners_in_world_space(
            width,
            height,
            0.001f,
            50.0f
        );

        // Bounds of what the camera can see (within reason)
        const std::array<glm::vec4, 8> bounds_real = camera.get_frustum_corners_in_world_space(
            width,
            height,
            0.001f,
            60.0f
        );

        // Find center of (shadowed) frustum
        glm::vec3 centre(0, 0, 0);
        for (const auto& b : bounds_shadowed)
            centre += glm::vec3(b);
        centre /= bounds_shadowed.size();

        // View matrix
        const glm::vec3 offset = glm::normalize(position) * 10.0f;
        const glm::mat4 light_view = glm::lookAt
        (
            centre + offset,            // position
            centre,                     // centre
            glm::vec3(0.0f, 1.0f, 0.0f) // up direction
        );

        // Work out corners of frustum from view matrix
        float min_x = std::numeric_limits<float>::max();
        float max_x = std::numeric_limits<float>::lowest();
        float min_y = std::numeric_limits<float>::max();
        float max_y = std::numeric_limits<float>::lowest();
        float min_z = std::numeric_limits<float>::max();
        float max_z = std::numeric_limits<float>::lowest();
        for (const auto& b : bounds_real)
        {
            const auto trf = light_view * b;
            min_x = std::min(min_x, trf.x);
            max_x = std::max(max_x, trf.x);
            min_y = std::min(min_y, trf.y);
            max_y = std::max(max_y, trf.y);
            min_z = std::min(min_z, trf.z);
            max_z = std::max(max_z, trf.z);
        }

        // "Pull back" slightly
        constexpr float z_pull_back = 10.0f;
        if (min_z < 0) min_z *= z_pull_back;
        else min_z /= z_pull_back;
        if (max_z < 0) max_z /= z_pull_back;
        else max_z *= z_pull_back;

        // Projection matrix
        const glm::mat4 light_projection = glm::ortho(min_x, max_x, min_y, max_y, min_z, max_z);

        return light_projection * light_view;
    }
};
