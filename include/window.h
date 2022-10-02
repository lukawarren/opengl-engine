#include <string>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

class Window
{
public:
    Window(const std::string& name, const int width, const int height);
    ~Window();

    bool update();

    int width;
    int height;

private:
    void init_glfw(const std::string& name);
    void init_glad();
    void enable_debugging();

    GLFWwindow* window;
};