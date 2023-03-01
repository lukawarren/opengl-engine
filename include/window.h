#pragma once
#include <string>
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class Window
{
public:
    Window(const std::string& name, const int width, const int height);
    ~Window();

    bool update();
    void set_title(const std::string& title) const;

    // I/O
    bool get_key(const int key) const;
    bool get_mouse_button(const int button, const bool repeat = true) const;
    glm::vec2 mouse_position() const;
    void capture_mouse() const;
    void uncapture_mouse() const;

    int width;
    int height;
    int framebuffer_width;
    int framebuffer_height;

private:
    void init_glfw(const std::string& name);
    void init_glad();
    void enable_debugging();

    GLFWwindow* window;
    bool cached_mouse_buttons[3] = {};
};