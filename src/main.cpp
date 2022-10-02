#include "window.h"
#include <iostream>
#include "mesh.h"

int main()
{
    std::vector<float> vertices = {
         0.5f,  0.5f, 0.0f,
         0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f,
        -0.5f,  0.5f, 0.0f
    };
    std::vector<unsigned int> indices = {
        0, 1, 3,
        1, 2, 3
    };

    Window window("nameless", 640, 480);
    Mesh mesh(vertices, indices);

    while (window.update())
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        mesh.bind();
        mesh.draw();
    }
    return 0;
}