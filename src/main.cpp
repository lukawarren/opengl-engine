#include "renderer.h"
#include <iostream>

constexpr int width = 1600;
constexpr int height = 900;

Scene terrain_scene();
Scene sponza_scene();

int main()
{
    Renderer renderer("New window", width, height, 1.0);
    Window& window = renderer.window;
    window.capture_mouse();

    // Movement state
    bool captured = true;
    glm::vec2 mouse_position = window.mouse_position();

    // Seutp scene
    Scene scene = sponza_scene();
    auto* camera = &scene.camera;
    double time = glfwGetTime();

    while (renderer.update(scene))
    {
        // Delta time
        const double new_time = glfwGetTime();
        const double delta = new_time - time;
        time = new_time;

        // Water waves
        for (auto& water : scene.waters) water.update(delta / 10.0f);

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
        const float speed = 4.0f * delta;
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
        glm::vec2 mouse_movement = window.mouse_position() - mouse_position;
        mouse_position = window.mouse_position();
        const float sensitivity = 0.1f;
        camera->yaw += mouse_movement.x * sensitivity;
        camera->pitch += mouse_movement.y * sensitivity;

        // Confine rotation
        camera->pitch = std::max(std::min(camera->pitch, 90.0f), -90.0f);
    }

    return 0;
}

Scene terrain_scene()
{
    Scene scene =
    {
        .entities = { Entity("cube.obj"), Entity("pool/pool.glb") },
        .waters = { Water() },
        .terrains = { Terrain("terrain/diffuse.png", "terrain/height_map.png") },
        .sun = { { 50.0f, 80.0f, 25.0f }, { 1.0f, 1.0f, 1.3f } },
        .camera = Camera({ -6.0f, 3.5f, 0.0f }, 30.0f, 90.0f, 0.0f)
    };

    auto* sponza = &scene.entities[2];
    auto* pool = &scene.entities[1];
    auto* water = &scene.waters[0];
    auto* terrain = &scene.terrains[0];

    pool->transform.scale = glm::vec3(5.0f);
    pool->transform.position = glm::vec3(0.0f, 0.1f, 0.0f);
    water->transform.scale = glm::vec3(4.3f);
    terrain->transform.position.y = -20;

    sponza->transform.scale = glm::vec3(0.03f);
    sponza->transform.position.y = -1;
    sponza->transform.position.z = 1;

    return scene;
}

Scene sponza_scene()
{
    Scene scene =
    {
        .entities = { Entity("sponza/sponza.gltf"), Entity("cube.obj") },
        .waters = {},
        .terrains = {},
        .sun = { { 0.15f, 1.0f, 0.3f }, { 255.0 / 255.0, 255.0 / 255.0, 200.0 / 255.0 } },
        .camera = Camera({ -6.0f, 3.5f, 0.0f }, 30.0f, 90.0f, 0.0f)
    };

    scene.sun.position *= 20.0f;
    scene.sun.colour *= 1.0f;

    auto* sponza = &scene.entities[0];
    sponza->transform.scale = glm::vec3(0.01f);
    sponza->transform.position.y = -1;
    sponza->transform.position.z = 1;
    sponza->transform.rotation.y = 90;

    auto* cube = &scene.entities[1];
    cube->transform.position.y = 5;

    return scene;
}