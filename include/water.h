#pragma once
#include "transform.h"
#include "framebuffer.h"
#include <memory>

constexpr unsigned int texture_width = 640;
constexpr unsigned int texture_height = 640;

class Water
{
public:
    Water(Transform _transform = {
        {},
        { 90.0f, 0.0f, 0.0f },
        { 20.0f, 20.0f, 20.0f }
    }) : transform(_transform)
    {
        reflection_buffer = std::make_shared<Framebuffer>(
            texture_width,
            texture_height
        );
    }

    Camera reflection_camera() const
    {
        return Camera(transform.position, -90.0f, 0.0f, 0.0f);
    }

    Transform transform;

    // To get around copy-constructor woes
    std::shared_ptr<Framebuffer> reflection_buffer;
};