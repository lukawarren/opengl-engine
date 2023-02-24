#include "renderer.h"
#include <iostream>

constexpr int width = 1600;
constexpr int height = 900;

Scene chunk_scene();
Scene sponza_scene();
void chunk_loop(Scene& scene);

int main()
{
    Renderer renderer("lukacraft", width, height, 1.0);
    Window& window = renderer.window;
    window.capture_mouse();

    // Movement state
    bool captured = true;
    glm::vec2 mouse_position = window.mouse_position();

    // Seutp scene
    Scene scene = chunk_scene();
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

        // Deal with mouse grabbing
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
        const float speed = 10.0f * delta;
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

        chunk_loop(scene);
    }

    return 0;
}

Scene chunk_scene()
{
    Scene scene =
    {
        .chunks = {},
        .entities = { Entity("monkey.obj") },
        .waters = { Water() },
        .skybox = MAKE_SKYBOX(
            "skybox/xpos.png",
            "skybox/xneg.png",
            "skybox/ypos.png",
            "skybox/yneg.png",
            "skybox/zpos.png",
            "skybox/zneg.png"
        ),
        .sun = { { 0.18f, 0.4f, 0.3f }, { 255.0 / 255.0, 255.0 / 255.0, 200.0 / 255.0 } },
        .camera = Camera({ 0.0f, 0.0f, 5.0f }, 30.0f, 0.0f, 0.0f)
    };

    // "Sky lighting"
    scene.skybox_tint = { 0.5f, 0.5f, 1.0f };
    scene.ambient_light = { 0.05f, 0.05f, 0.1f };

    scene.sun.position *= 50.0f;
    scene.sun.colour *= 0.8f;

    auto& monkey = scene.entities[0];
    monkey.transform.position = { 15, 15, 15 };

    auto& water = scene.waters[0];
    water.transform.scale *= 100.0f;
    water.transform.position.y = 5.0f;

    water.transform.position.x = Chunk::size * 2;
    water.transform.position.z = Chunk::size * 2;
    scene.camera.position.x = Chunk::size * 2;
    scene.camera.position.z = Chunk::size * 2;
    scene.camera.position.y = 20;

    for (int x = 0; x < 4; ++x)
        for (int z = 0; z < 4; ++z)
            scene.chunks.emplace_back(glm::ivec3 {x, 0, z});

    return scene;
}

Scene sponza_scene()
{
    Scene scene =
    {
        .entities = { Entity("sponza/sponza.gltf"), Entity("cube.obj") },
        .waters = {},
        .sun = { { 0.18f, 1.0f, 0.3f }, { 255.0 / 255.0, 255.0 / 255.0, 200.0 / 255.0 } },
        .camera = Camera({ -6.0f, 3.5f, 0.0f }, 30.0f, 90.0f, 0.0f)
    };

    scene.sun.position *= 20.0f;
    scene.sun.colour *= 1.5f;

    auto* sponza = &scene.entities[0];
    sponza->transform.scale = glm::vec3(0.01f);
    sponza->transform.position.y = -1;
    sponza->transform.position.z = 1;
    sponza->transform.rotation.y = 90;

    auto* cube = &scene.entities[1];
    cube->transform.position.y = 5;

    return scene;
}

void chunk_loop(Scene& scene)
{
    const glm::vec3 direction = scene.camera.direction_vector();
    const glm::vec3 origin = scene.camera.position;
    const int max_distance = 100;

    for (int d = 0; d < max_distance; ++d)
    {
        // March along
        glm::vec3 ray_pos = origin + direction * (float)d;

        // Locate chunk
        int chunk_x = int(ray_pos.x / (float)Chunk::size);
        int chunk_z = int(ray_pos.z / (float)Chunk::size);
        if (chunk_x >= 4 || chunk_z >= 4) break;
        auto& chunk = scene.chunks[chunk_x * 4 + chunk_z];

        // Find block
        int block_x = (int)(ray_pos.x + 0.5f) % Chunk::size;
        int block_y = (int)(ray_pos.y + 0.5f) % Chunk::max_height;
        int block_z = (int)(ray_pos.z + 0.5f) % Chunk::size;

        auto& block = chunk.blocks[block_x][block_y][block_z];
        if (block == Block::Air) continue;
        block= Block::Wood;
        chunk.rebuild_mesh();
        break;
    }
}