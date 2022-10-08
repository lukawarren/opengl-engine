#include "renderer.h"
#include <iostream>

constexpr int width = 800;
constexpr int height = 600;

int main()
{
    Renderer renderer("New window", width, height, 0.4);
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
        .camera = Camera({ 0.0f, 5.5f, 4.0f }, 10.0f, 0.0f, 0.0f)
    };

    auto* sponza = &scene.entities[0];
    auto* water = &scene.waters[0];
    auto* camera = &scene.camera;

    sponza->transform.scale = glm::vec3(0.1f);
    sponza->transform.position.y = -10;
    water->transform.position.y = -1;

    while (renderer.update(scene))
    {
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
        glm::vec3 speed = {};
        if (window.get_key(GLFW_KEY_W)) speed.z += 1.0f;
        if (window.get_key(GLFW_KEY_S)) speed.z -= 1.0f;
        if (window.get_key(GLFW_KEY_A)) speed.x -= 1.0f;
        if (window.get_key(GLFW_KEY_D)) speed.x += 1.0f;

        // Apply relative to rotation
        camera->position += glm::vec3 { sin(glm::radians(camera->yaw)), 0, -cos(glm::radians(camera->yaw)) } * speed.z * 1.0f;
        camera->position += glm::vec3 { cos(glm::radians(camera->yaw)), 0,  sin(glm::radians(camera->yaw)) } * speed.x * 1.0f;

        // Vertical movement
        if (window.get_key(GLFW_KEY_SPACE)) camera->position.y += 1.0f;
        if (window.get_key(GLFW_KEY_LEFT_SHIFT)) camera->position.y -= 1.0f;

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