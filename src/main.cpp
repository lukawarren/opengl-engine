#include "window.h"
#include <iostream>
#include "mesh.h"
#include "shader.h"
#include "texture.h"
#include "camera.h"

constexpr int width = 640;
constexpr int height = 480;

int main()
{
    Window window("nameless", width, height);

    Texture texture("duck.jpg");
    texture.bind();

    Mesh mesh = Mesh::quad();

    Shader shader("basic");
    shader.bind();

    Camera camera({ 0.0f, 1.0f, 2.0f }, 10.0f, 0.0f, 0.0f);
    shader.set_uniform("view", camera.view_matrix());
    shader.set_uniform("projection", camera.projection_matrix(
        width, height
    ));

    while (window.update())
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mesh.bind();
        mesh.draw();
    }
    return 0;
}