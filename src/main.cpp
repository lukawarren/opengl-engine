#include "renderer.h"
#include <iostream>

constexpr int width = 800;
constexpr int height = 600;

int main()
{
    Renderer renderer("New window", width, height);

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

    float t = 0.0f;
    while (renderer.update(scene))
    {
        camera->position.z = std::cos(t += 0.01f) * 10 + 10;
    }

    return 0;
}