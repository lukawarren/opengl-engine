#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <array>

constexpr float z_near = 0.01f;
constexpr float z_far = 1000.0f;
constexpr float fov = glm::radians(90.0f);

class Camera
{
public:
    Camera() {}

    Camera(glm::vec3 _position, float _pitch, float _yaw, float _roll) :
        position(_position), pitch(_pitch), yaw(_yaw), roll(_roll) {}

    glm::mat4 view_matrix() const
    {
        glm::mat4 view = glm::mat4(1.0f);

        // Rotation
        view = glm::rotate(view, glm::radians(pitch), glm::vec3(1, 0, 0));
        view = glm::rotate(view, glm::radians(yaw),   glm::vec3(0, 1, 0));
        view = glm::rotate(view, glm::radians(roll),  glm::vec3(0, 0, 1));

        // Translation
        view = glm::translate(view, -position);
        return view;
    }

    glm::mat4 projection_matrix(
        const float width,
        const float height,
        const float near = z_near,
        const float far = z_far
    ) const
    {
        return glm::perspective(fov, width / height, near, far);
    }

    glm::vec3 direction_vector() const
    {
        // Eye coordinates at centre of screen
        glm::vec4 eye = { 0.0f, 0.0f, -1.0f, 0.0f };

        // World coordinates
        glm::vec4 world_ray_xyzw = glm::inverse(view_matrix()) * eye;
        glm::vec3 world_ray_xyz = { world_ray_xyzw.x, world_ray_xyzw.y, world_ray_xyzw.z };
        return glm::normalize(world_ray_xyz);
    }

    std::array<glm::vec4, 8> get_frustum_corners_in_world_space(
        const float width,
        const float height,
        const float near = z_near,
        const float far = z_far
    ) const
    {
        const auto proj = projection_matrix(width, height, near, far);
        const auto view = view_matrix();
        const auto inv = glm::inverse(proj * view);
        std::array<glm::vec4, 8> frustum_corners;

        for (unsigned int x = 0; x < 2; ++x)
        {
            for (unsigned int y = 0; y < 2; ++y)
            {
                for (unsigned int z = 0; z < 2; ++z)
                {
                    const glm::vec4 pt = inv * glm::vec4(
                        2.0f * x - 1.0f,
                        2.0f * y - 1.0f,
                        2.0f * z - 1.0f,
                        1.0f
                    );

                    frustum_corners[x * 4 + y * 2 + z] = pt / pt.w;
                }
            }
        }

        return frustum_corners;
    }

    glm::vec3 position = {};
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
};