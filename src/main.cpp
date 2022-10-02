#include "window.h"
#include <iostream>
#include "mesh.h"
#include "shader.h"
#include "texture.h"

int main()
{
    Window window("nameless", 640, 480);
    Texture texture("duck.jpg");
    texture.bind();
    Mesh mesh = Mesh::quad();
    Shader shader("basic");
    shader.bind();

    while (window.update())
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mesh.bind();
        mesh.draw();
    }
    return 0;
}