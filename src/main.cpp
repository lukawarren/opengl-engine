#include "renderer.h"
#include <iostream>

constexpr int width = 1600;
constexpr int height = 900;

int main()
{
    Renderer renderer("New window", width, height, 1);
    Window& window = renderer.window;
    window.capture_mouse();

    // Movement state
    bool captured = true;
    glm::vec2 mouse_position = window.mouse_position();

    // Seutp scene
    Scene scene =
    {
        .entities = { Entity("sponza/sponza.gltf") },
        .waters = { Water() },
        .camera = Camera({ 0.0f, 10.5f, 4.0f }, 10.0f, 0.0f, 0.0f)
    };

    auto* sponza = &scene.entities[0];
    auto* water = &scene.waters[0];
    auto* camera = &scene.camera;

    sponza->transform.scale = glm::vec3(0.01f);
    sponza->transform.position.y = 4;
    sponza->transform.position.z = 3;
    water->transform.position.y = 10;

    while (renderer.update(scene))
    {
        // Water waves
        water->update();

        // Deal with mouse grabbinig
        if (window.get_key(GLFW_KEY_ESCAPE))
        {
            window.uncapture_mouse();
            captured = false;
        }

        if (window.get_mouse_button(GLFW_MOUSE_BUTTON_LEFT))
        {
            window.capture_mouse();
            mouse_position = window.mouse_position();
            captured = true;
        }

        if (!captured) continue;

        // WASD
        const float speed = 0.04f;
        glm::vec3 movement = {};
        if (window.get_key(GLFW_KEY_W)) movement.z += 1.0f;
        if (window.get_key(GLFW_KEY_S)) movement.z -= 1.0f;
        if (window.get_key(GLFW_KEY_A)) movement.x -= 1.0f;
        if (window.get_key(GLFW_KEY_D)) movement.x += 1.0f;

        // Apply relative to rotation
        camera->position += glm::vec3 { sin(glm::radians(camera->yaw)), 0, -cos(glm::radians(camera->yaw)) } * movement.z * speed;
        camera->position += glm::vec3 { cos(glm::radians(camera->yaw)), 0,  sin(glm::radians(camera->yaw)) } * movement.x * speed;

        // Vertical movement
        if (window.get_key(GLFW_KEY_SPACE)) camera->position.y += 1.0f * speed;
        if (window.get_key(GLFW_KEY_LEFT_SHIFT)) camera->position.y -= 1.0f * speed;

        // Mouse
        glm::vec2 delta = window.mouse_position() - mouse_position;
        mouse_position = window.mouse_position();
        const float sensitivity = 0.1f;
        camera->yaw += delta.x * sensitivity;
        camera->pitch += delta.y * sensitivity;

        // Confine rotation
        camera->pitch = std::max(std::min(camera->pitch, 90.0f), -90.0f);
    }

    return 0;
}