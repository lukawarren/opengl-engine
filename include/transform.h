#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform
{
public:

    Transform(const glm::vec3 position = {}, const glm::vec3 rotation = {},
        const glm::vec3 scale = { 1.0f, 1.0f, 1.0f })
    {
        this->position = position;
        this->rotation = rotation;
        this->scale =scale;
    }

    glm::mat4 matrix() const
    {
        glm::mat4 matrix = glm::mat4(1.0f);

        // Translation
        matrix = glm::translate(matrix, position);

        // Rotation
        matrix = glm::rotate(matrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
        matrix = glm::rotate(matrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
        matrix = glm::rotate(matrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));

        // Scale
        matrix = glm::scale(matrix, { scale.x, scale.y, scale.z });
        return matrix;
    }

    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};
