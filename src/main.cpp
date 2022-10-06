#include "window.h"
#include "renderer.h"
#include "camera.h"
#include "entity.h"
#include "texture.h"
#include <iostream>

constexpr int width = 800;
constexpr int height = 600;

int main()
{
    Renderer renderer("New window", width, height);
    Camera camera({ 0.0f, 1.5f, 4.0f }, 10.0f, 0.0f, 0.0f);

    std::vector<Entity> entities = {
        Entity("sponza/sponza.gltf"),
    };

    std::vector<Water> waters = {
        Water()
    };

    auto* sponza = &entities[0];
    auto* water = &waters[0];

    sponza->transform.scale = glm::vec3(0.1f);
    sponza->transform.position.y = -10;
    water->transform.position.y = -1;

    float t = 0.0f;
    while (renderer.update(entities, waters, camera))
    {
        camera.position.z = std::cos(t += 0.01f) * 10 + 10;
        sponza->transform.rotation.y += 0.1f;
    }

    return 0;
}