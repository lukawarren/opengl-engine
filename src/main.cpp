#include "renderer.h"
#include <iostream>

constexpr int width = 1600;
constexpr int height = 900;

Scene chunk_scene();
Scene sponza_scene();
void chunk_loop(Scene& scene, const Window& window);

int main()
{
    Renderer renderer("gl", width, height, RENDER_SCALE);
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

        // Moving clouds
        scene.cloud_settings.time += delta * 5.0f;

        // Deal with mouse grabbing
        if (window.get_key(GLFW_KEY_ESCAPE, false))
        {
            if (captured)
            {
                window.uncapture_mouse();
                captured = false;
            }
            else
            {
                window.capture_mouse();
                mouse_position = window.mouse_position();
                captured = true;
            }
        }

        if (!captured) continue;

        // WASD
        const float speed = 30.0f * delta;
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

        //chunk_loop(scene, window);
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
        .sprites = { Sprite("crosshair.png") },
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

    // Lighting
    scene.skybox_tint = { 0.5f, 0.5f, 1.0f };
    scene.ambient_light = { 0.05f, 0.05f, 0.1f };
    scene.sun.position *= 1000.0f;
    scene.sun.colour *= 0.8f;

    auto& monkey = scene.entities[0];
    monkey.transform.position = { 15, 15, 15 };

    auto& crosshair = scene.sprites[0];
    crosshair.transform.scale = glm::vec3(0.02f);

    // Water and positioning
    auto& water = scene.waters[0];
    water.transform.scale *= 100.0f;
    water.transform.position.y = 5.0f;
    water.transform.position.x = Chunk::size * 2;
    water.transform.position.z = Chunk::size * 2;
    scene.camera.position.x = Chunk::size * 2;
    scene.camera.position.z = Chunk::size * 2;
    scene.camera.position.y = 20;

    // Chunks
    for (int x = 0  ; x < 4; ++x)
        for (int z = 0; z < 4; ++z)
            scene.chunks.emplace_back(glm::ivec3 {x, 0, z});

    return scene;
}

Scene sponza_scene()
{
    Scene scene =
    {
        .entities = { Entity("sponza/sponza.gltf"), Entity("cube.obj") },
        .waters = { Water() },
        .skybox = MAKE_SKYBOX(
            "skybox/xpos.png",
            "skybox/xneg.png",
            "skybox/ypos.png",
            "skybox/yneg.png",
            "skybox/zpos.png",
            "skybox/zneg.png"
        ),
        .sun = { { 0.18f, 1.0f, 0.3f }, { 255.0 / 255.0, 255.0 / 255.0, 200.0 / 255.0 } },
        .camera = Camera({ -6.0f, 3.5f, 0.0f }, 30.0f, 90.0f, 0.0f),
    };

    scene.cloud_settings.enabled = false;

    // Lighting
    scene.sun.position *= 20.0f;
    scene.sun.colour *= 2.5f;

    // Position sponza model
    auto* sponza = &scene.entities[0];
    sponza->transform.scale = glm::vec3(0.01f);
    sponza->transform.position.y = -1;
    sponza->transform.position.z = 1;
    sponza->transform.rotation.y = 90;

    // Water and positioning
    auto& water = scene.waters[0];
    water.transform.scale *= 100.0f;
    water.transform.position = { 0.0f, 0.0f, 0.0f };

    return scene;
}

void chunk_loop(Scene& scene, const Window& window)
{
    const glm::vec3 direction = scene.camera.direction_vector();
    const glm::vec3 origin = scene.camera.position;
    const int max_distance = 7;
    const int substeps = 16;

    const auto get_block_pos = [&](const glm::vec3 ray_pos)
    {
        // Locate chunk
        int chunk_x = int(ray_pos.x / (float)Chunk::size);
        int chunk_z = int(ray_pos.z / (float)Chunk::size);
        if (chunk_x >= 4 || chunk_z >= 4 || chunk_x < 0 || chunk_z < 0)
            return std::optional<BlockPosition> {};

        int chunk_id = chunk_x * 4 + chunk_z;

        // Find block
        int block_x = (int)(ray_pos.x) % Chunk::size;
        int block_y = (int)(ray_pos.y) % Chunk::max_height;
        int block_z = (int)(ray_pos.z) % Chunk::size;

        if (block_x >= Chunk::size || block_y >= Chunk::max_height || block_z >= Chunk::size
            || block_x < 0 || block_y < 0 || block_z < 0)
            return std::optional<BlockPosition> {};

        return std::optional<BlockPosition> {{ chunk_id, block_x, block_y, block_z }};
    };

    for (int d = 1; d < max_distance * substeps; ++d)
    {
        // March along
        glm::vec3 ray_pos = origin + direction * (float)d  / (float)substeps;
        ray_pos = glm::round(ray_pos);

        // Convert ray to block position within chunk
        auto block_pos = get_block_pos(ray_pos);
        if (!block_pos.has_value()) continue;

        // Ignore if block is air
        auto& chunk = scene.chunks[block_pos->chunk];
        auto& block = chunk.blocks[block_pos->x][block_pos->y][block_pos->z];
        if (block == Block::Air) continue;

        // --- Solid block found; do as we please ---

        // Breaking blocks
        if (window.get_mouse_button(GLFW_MOUSE_BUTTON_LEFT, false))
        {
            block = Block::Air;
            chunk.rebuild_mesh();
        }

        // Placing blocks
        if (window.get_mouse_button(GLFW_MOUSE_BUTTON_RIGHT, false))
        {
            // Current block is solid, so as long as the previous
            // one is air, we're fine
            glm::vec3 previous_ray = origin + direction * (float)(d-1) / (float)substeps;
            previous_ray = glm::round(previous_ray);

            auto previous_block_pos = get_block_pos(previous_ray);
            if (!previous_block_pos.has_value()) break;

            auto& previous_chunk = scene.chunks[previous_block_pos->chunk];
            auto& previous_block = previous_chunk.blocks[previous_block_pos->x][previous_block_pos->y][previous_block_pos->z];

            previous_block = Block::Leaves;
            previous_chunk.rebuild_mesh();
        }

        break;
    }
}
