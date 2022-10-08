#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Macros to help with verbosity
#define SHADER_UNIFORM(x) void Shader::set_uniform(const std::string& name, x)
#define _SHADER_UNIFORM(x) void set_uniform(const std::string& name, x)
#define NAME get_uniform_location(name)

class Shader
{
public:
    Shader(const std::string& filename);
    Shader(const Shader&) = delete;
    ~Shader();

    void bind() const;
    void unbind() const;

    _SHADER_UNIFORM(const glm::mat4& matrix);
    _SHADER_UNIFORM(const glm::vec4& vector);
    _SHADER_UNIFORM(const glm::vec3& vector);
    _SHADER_UNIFORM(const glm::vec2& vector);
    _SHADER_UNIFORM(const float value);
    _SHADER_UNIFORM(const int value);

private:

    void handle_error(auto f, auto f2, const unsigned int id,
        const unsigned int type, const std::string& error, const std::string& filename);

    int get_uniform_location(const std::string& name);

    // OpenGL state
    unsigned int program;
    std::unordered_map<std::string, int> uniforms;
};

class DiffuseShader : public Shader
{
public:
    DiffuseShader() : Shader("diffuse") {}
};

class WaterShader : public Shader
{
public:
    WaterShader() : Shader("water") {}
};
