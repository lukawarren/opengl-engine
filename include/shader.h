#include <array>
#include <string>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Macros to help with verbosity
#define SHADER_UNIFORM(x) void Shader::set_uniform(const std::string& name, x)
#define DEFINE_SHADER_UNIFORM(x) void set_uniform(const std::string& name, x)
#define NAME get_uniform_location(name)

class Shader
{
public:
    struct ShaderType
    {
        std::string extension;
        unsigned int identifier;
    };

    enum ShaderTypeID
    {
        Vertex = 0,
        Fragment = 1,
        Compute = 2
    };

    static std::array<ShaderType, 3> shader_types;

public:
    Shader(const std::string& filename, const std::vector<ShaderTypeID>& shader_type_ids);
    Shader(const Shader&) = delete;
    ~Shader();

    void bind() const;
    void unbind() const;

    DEFINE_SHADER_UNIFORM(const glm::mat4& matrix);
    DEFINE_SHADER_UNIFORM(const glm::vec4& vector);
    DEFINE_SHADER_UNIFORM(const glm::vec3& vector);
    DEFINE_SHADER_UNIFORM(const glm::vec2& vector);
    DEFINE_SHADER_UNIFORM(const float value);
    DEFINE_SHADER_UNIFORM(const int value);

private:

    void handle_error(auto f, auto f2, const unsigned int id,
        const unsigned int type, const std::string& error, const std::string& filename);

    int get_uniform_location(const std::string& name);

    // OpenGL state
    unsigned int program;
    std::unordered_map<std::string, int> uniforms;
};

// Shader classes
#define SHADER(x, y, z) class x : public Shader {\
public:\
    x() : Shader(y, z) {}\
};

#define SHADER_NORMAL {\
    ShaderTypeID::Vertex,\
    ShaderTypeID::Fragment\
}

SHADER(DiffuseShader,   "diffuse",      SHADER_NORMAL)
SHADER(WaterShader,     "water",        SHADER_NORMAL)
SHADER(QuadShader,      "quad",         SHADER_NORMAL)
SHADER(ShadowMapShader, "shadow_map",   SHADER_NORMAL)
SHADER(CompositeShader, "composite",    SHADER_NORMAL)
SHADER(BlurShader,      "blur",         SHADER_NORMAL)
SHADER(BloomShader,     "bloom",        SHADER_NORMAL)
SHADER(SkyboxShader,    "skybox",       SHADER_NORMAL)
SHADER(CloudShader,     "cloud",        SHADER_NORMAL)
SHADER(WorleyShader,    "worley",       { ShaderTypeID::Compute })
