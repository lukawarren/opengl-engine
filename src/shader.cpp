#include "shader.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <glad/glad.h>

std::array<Shader::ShaderType, 3> Shader::shader_types =
{{
    { .extension = ".vert", .identifier = GL_VERTEX_SHADER   },
    { .extension = ".frag", .identifier = GL_FRAGMENT_SHADER },
    { .extension = ".comp", .identifier = GL_COMPUTE_SHADER  }
}};

Shader::Shader(const std::string& filename, const std::vector<ShaderTypeID>& shader_type_ids)
{
    const auto read_file = [](const std::string path)
    {
        std::ifstream in("../res/shaders/" + path);
        std::string contents((std::istreambuf_iterator<char>)(in), std::istreambuf_iterator<char>());
        if (contents == "") throw std::runtime_error("unable to load shader " + path);
        return contents;
    };

    struct ShaderTarget
    {
        std::string source;
        unsigned int shader;
        ShaderTypeID type_id;
    };

    // Load from disk
    std::vector<ShaderTarget> targets;
    for (const ShaderTypeID id : shader_type_ids)
    {
        targets.emplace_back(ShaderTarget {
            .source = read_file(filename + shader_types[id].extension),
            .shader = 0,
            .type_id = id
        });
    }

    // Upload source code
    for (auto& target : targets)
    {
        const char* c_str = target.source.c_str();
        target.shader = glCreateShader(shader_types[target.type_id].identifier);
        glShaderSource(target.shader, 1, &c_str, NULL);
    }

    // Compile and check for errors
    for (auto& target : targets)
    {
        const std::string& extension = shader_types[target.type_id].extension;
        glCompileShader(target.shader);
        handle_error(
            glGetShaderiv,
            glGetShaderInfoLog,
            target.shader,
            GL_COMPILE_STATUS,
            extension + " shader failed to compile",
            filename
        );
    }

    // Link into one "program"
    program = glCreateProgram();
    for (auto& target : targets)
        glAttachShader(program, target.shader);
    glLinkProgram(program);
    handle_error(glGetProgramiv, glGetProgramInfoLog, program, GL_LINK_STATUS, "shader failed to link", filename);

    // Shaders themselves no longer needed - all we need is the final program
    for (auto& target : targets)
        glDeleteShader(target.shader);
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
SHADER_UNIFORM(const glm::vec4& vector) { glUniform4f(NAME, vector.x, vector.y, vector.z, vector.w); }
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

int Shader::get_uniform_location(const std::string& name)
{
    // Use cached version...
    if (uniforms.contains(name))
        return uniforms.at(name);

    // ...or get from OpenGL then store for later
    int result = glGetUniformLocation(program, name.c_str());
    if (result < 0) std::cout << "failed to create uniform "  << name << std::endl;
    uniforms.emplace(name, result);
    return result;
}

Shader::~Shader()
{
    glDeleteProgram(program);
}