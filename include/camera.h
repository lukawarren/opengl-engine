#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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

    glm::mat4 projection_matrix(const float width, const float height) const
    {
        return glm::perspective(fov, width / height, z_near, z_far);
    }

    glm::vec3 position = {};
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 0.0f;
};