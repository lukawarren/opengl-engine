#pragma once
#include "transform.h"
#include "framebuffer.h"
#include <memory>

constexpr unsigned int texture_width = 256;
constexpr unsigned int texture_height = 256;

class Water
{
public:
    Water(Transform _transform = {
        {},
        { 90.0f, 0.0f, 0.0f },
        { 100.0f, 100.0f, 100.0f }
    }) : transform(_transform)
    {
        reflection_buffer = std::make_shared<Framebuffer>(texture_width, texture_height);
        refraction_buffer = std::make_shared<Framebuffer>(texture_width, texture_height);
    }

    // Computes where the camera should face to render a planar reflection
    // as seen from the incoming camera
    Camera reflection_camera(const Camera& camera) const
    {
        glm::vec3 position = camera.position;
        position.y -= 2.0f * (camera.position.y - transform.position.y);

        return Camera(
            position,
            -camera.pitch,
            camera.yaw,
            camera.roll
        );
    }

    Transform transform;

    // To get around copy-constructor woes
    std::shared_ptr<Framebuffer> reflection_buffer;
    std::shared_ptr<Framebuffer> refraction_buffer;
};