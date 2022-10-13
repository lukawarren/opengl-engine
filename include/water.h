#pragma once
#include "transform.h"
#include "framebuffer.h"
#include "resources.h"
#include <memory>

constexpr unsigned int texture_width = 1024;
constexpr unsigned int texture_height = 1024;

class Water
{
public:
    Water(Transform _transform = {
        {},
        { 90.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 1.0f }
    }) : transform(_transform)
    {
        reflection_buffer = std::make_shared<Framebuffer>(texture_width, texture_height);
        refraction_buffer = std::make_shared<Framebuffer>(texture_width, texture_height, true);
        distortion_map = get_texture("water/dudv.png");
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

    void update()
    {
        time += 0.0008f / 2.0f;
    }

    Transform transform;
    float time = 0.0f;

    // To get around copy-constructor woes
    std::shared_ptr<Framebuffer> reflection_buffer;
    std::shared_ptr<Framebuffer> refraction_buffer;
    Texture* distortion_map;
};