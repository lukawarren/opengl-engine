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
    Texture texture("duck.jpg");
    texture.bind();

    std::vector<Entity> entities = {
        Entity("cube.obj"),
        Entity("cube.obj")
    };

    Camera camera({ 0.0f, 1.5f, 4.0f }, 10.0f, 0.0f, 0.0f);

    float t = 0.0f;
    while (renderer.update(entities, camera)) {
        camera.position.z = std::cos(t += 0.01f) * 10 + 10;
    }

    return 0;
}