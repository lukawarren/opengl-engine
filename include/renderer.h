#pragma once
#include "window.h"
#include "shader.h"
#include "scene.h"
#include <string>
#include <array>
#include <optional>

class Renderer
{
public:
    Renderer(const std::string& title, const int width, const int height, const float render_scale);
    ~Renderer();

    bool update(const Scene& scene);

    Window window;

private:

    // Render passes
    void diffuse_pass(
        const Scene& scene,
        const Camera& camera,
        const unsigned int width,
        const unsigned int height,
        const std::optional<glm::vec4> clip_plane = {});
    void water_pass(const Scene& scene);
    void shadow_pass(const Scene& scene);
    void bloom_pass();
    void blur_pass(const Texture& texture, const Framebuffer& final_framebuffer);
    void sprite_pass(const Scene& scene);
    void cloud_pass(const Scene& scene);

    // Output
    float render_scale;
    Framebuffer bloom_framebuffer;
    Framebuffer blur_framebuffers[2];
    Framebuffer output_framebuffer;
    Framebuffer clouds_framebuffer;
    unsigned int render_width() const;
    unsigned int render_height() const;

    // Shaders
    DiffuseShader diffuse_shader;
    WaterShader water_shader;
    QuadShader quad_shader;
    ShadowMapShader shadow_shader;
    CompositeShader composite_shader;
    BlurShader blur_shader;
    BloomShader bloom_shader;
    SkyboxShader skybox_shader;
    CloudShader cloud_shader;
    WorleyShader worley_shader;

    // Cloud rendering
    Texture* cloud_noises[2];
    void init_clouds(const float scale = 6.0f);

    bool did_bake_shadows = false;

    // FPS counting
    double last_fps_report_time = 0.0f;
};