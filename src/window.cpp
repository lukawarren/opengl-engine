#include "window.h"
#include <stdexcept>
#include <iostream>

Window::Window(const std::string& name, const int _width, const int _height) :
    width(_width), height(_height)
{
    // Create window and accompanying OpenGL context
    init_glfw(name);
    init_glad();

#ifndef __APPLE__
    // On modern platforms (i.e. not Apple), we can enable GL_DEBUG_OUTPUT
    // to catch errors that might otherwise go unnoticed.
    enable_debugging();
#endif
}

bool Window::update()
{
    glfwSwapBuffers(window);
    glfwPollEvents();
    return !glfwWindowShouldClose(window);
}

void Window::init_glfw(const std::string& name)
{
    // Init OpenGL core (>= 4.3 for debugging output)
    glfwInit();
#ifndef __APPLE__
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#else
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
#endif
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, true);

    // Create window
    window = glfwCreateWindow(width, height, name.c_str(), NULL, NULL);
    if (!window) throw std::runtime_error("unable to create GLFW window");

    // Get *framebuffer* size (may differ on retina displays, etc.)
    glfwGetFramebufferSize( window, &framebuffer_width, &framebuffer_height);

    // Bind window to OpenGL context and enable vsync
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
}

void Window::init_glad()
{
    // GLAD will load OpenGL for us
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        throw std::runtime_error("failed to initialise GLAD");
}

void Window::enable_debugging()
{
    // Load debug context synchronously (i.e. raise errors as soon as they happen)
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);

    glDebugMessageCallback([]
    (
        GLenum source,
        GLenum type,
        unsigned int id,
        GLenum severity,
        GLsizei,
        const char* message,
        const void*
        )
    {
        // Ignore certain "non-errors" (like Nvidia's "buffer successfully created")
        if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

        std::cerr << "[Debug] ID " << id << std::endl;

        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             std::cerr << "Source: API";                   break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cerr << "Source: Window System";         break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cerr << "Source: Shader Compiler";       break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cerr << "Source: Third Party";           break;
            case GL_DEBUG_SOURCE_APPLICATION:     std::cerr << "Source: Application";           break;
            case GL_DEBUG_SOURCE_OTHER:           std::cerr << "Source: Other";                 break;
            default:                              std::cerr << "Source: Unknown";               break;
        }

        std::cerr << std::endl;

        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:               std::cerr << "Type: Error";                 break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cerr << "Type: Deprecated Behaviour";  break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cerr << "Type: Undefined Behaviour";   break;
            case GL_DEBUG_TYPE_PORTABILITY:         std::cerr << "Type: Portability";           break;
            case GL_DEBUG_TYPE_PERFORMANCE:         std::cerr << "Type: Performance";           break;
            case GL_DEBUG_TYPE_MARKER:              std::cerr << "Type: Marker";                break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          std::cerr << "Type: Push Group";            break;
            case GL_DEBUG_TYPE_POP_GROUP:           std::cerr << "Type: Pop Group";             break;
            case GL_DEBUG_TYPE_OTHER:               std::cerr << "Type: Other";                 break;
            default:                                std::cerr << "Type: Unknown";               break;
        }

        std::cerr << std::endl;

        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         std::cerr << "Severity: high";                 break;
            case GL_DEBUG_SEVERITY_MEDIUM:       std::cerr << "Severity: medium";               break;
            case GL_DEBUG_SEVERITY_LOW:          std::cerr << "Severity: low";                  break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: std::cerr << "Severity: notification";         break;
            default:                             std::cerr << "Severity: Unknown";              break;
        }

        std::cerr << std::endl;
        std::cerr << "Message: " << message << std::endl << std::endl;
    }, nullptr);

    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
}

Window::~Window()
{
    glfwTerminate();
}
