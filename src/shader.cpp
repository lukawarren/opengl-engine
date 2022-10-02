#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <glad/glad.h>

Shader::Shader(const std::string& filename)
{
    const auto read_file = [](const std::string path)
    {
        std::ifstream in("../res/shaders/" + path);
        std::string contents((std::istreambuf_iterator<char>)(in), std::istreambuf_iterator<char>());
        if (contents == "") throw std::runtime_error("unable to load shader " + path);
        return contents;
    };

    // Load from disk
    const std::string vertex_source = read_file(filename + ".vert");
    const std::string fragment_source = read_file(filename + ".frag");
    const char* c_vertex = vertex_source.c_str();
    const char* c_fragment = fragment_source.c_str();

    // Create and upload source code
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vertex_shader, 1, &c_vertex, NULL);
    glShaderSource(fragment_shader, 1, &c_fragment, NULL);

    // Compile and checkk for errors
    glCompileShader(vertex_shader);
    glCompileShader(fragment_shader);
    handle_error(glGetShaderiv, glGetShaderInfoLog, vertex_shader, GL_COMPILE_STATUS, "vertex shader failed to compile", filename);
    handle_error(glGetShaderiv, glGetShaderInfoLog, fragment_shader, GL_COMPILE_STATUS, "fragment shader failed to compile", filename);

    // Link into one "program"
    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    handle_error(glGetProgramiv, glGetProgramInfoLog, program, GL_LINK_STATUS, "shader failed to link", filename);

    // Shaders themselves no longer needed - all we need is the final program
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::bind() const
{
    glUseProgram(program);
}

void Shader::unbind() const
{
    glUseProgram(0);
}


SHADER_UNIFORM(const glm::mat4& matrix) { glUniformMatrix4fv(NAME, 1, GL_FALSE, glm::value_ptr(matrix)); }
SHADER_UNIFORM(const glm::vec3& vector) { glUniform3f(NAME, vector.x, vector.y, vector.z); }
SHADER_UNIFORM(const glm::vec2& vector) { glUniform2f(NAME, vector.x, vector.y); }
SHADER_UNIFORM(const float value)       { glUniform1f(NAME, value); }
SHADER_UNIFORM(const int value)         { glUniform1i(NAME, value); }

void Shader::handle_error(auto f, auto f2, const unsigned int id,
        const unsigned int type, const std::string& error, const std::string& filename)
{
    int success;
    f(id, type, &success);
    if (!success)
    {
        char info[512] = {};
        f2(id, 512, NULL, info);
        throw std::runtime_error(filename + ": " + error + "\n" + info);
    }
}

Shader::~Shader()
{
    glDeleteProgram(program);
}