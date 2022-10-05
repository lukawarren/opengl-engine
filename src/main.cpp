#include "window.h"
#include <iostream>
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"
#include "entity.h"

constexpr int width = 640;
constexpr int height = 480;

int main()
{
    Window window("nameless", width, height);

    Texture texture("duck.jpg");
    texture.bind();

    Entity entity = Entity("cube.obj");

    Shader shader("basic");
    shader.bind();

    Camera camera({ 0.0f, 1.5f, 4.0f }, 10.0f, 0.0f, 0.0f);
    shader.set_uniform("view", camera.view_matrix());
    shader.set_uniform("projection", camera.projection_matrix(
        width, height
    ));

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    while (window.update())
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        for (const auto& mesh : entity.meshes)
        {
            mesh->bind();
            mesh->draw();
        }
    }
    return 0;
}