#pragma once
#include "../shader.h"
#include "../scene.h"

class RenderPass
{
public:
    RenderPass() {}
    virtual ~RenderPass() {};
};

class GBufferPass : public RenderPass
{
public:
    GBufferPass(const unsigned int width, const unsigned int height);
    void render(
        const Scene& scene,
        const glm::mat4& view,
        const glm::mat4& projection,
        const std::optional<glm::vec4> clip_plane = {}
    );
    Framebuffer g_buffer;

private:
    GBufferShader shader;
};

class LightingPass : public RenderPass
{
public:
    LightingPass(const unsigned int width, const unsigned int height);
    void render(
        const Scene& scene,
        const glm::mat4& light_projection,
        const Texture* cloud_noise,
        const Texture& shadow_map,
        const Framebuffer& g_buffer
    );
    Framebuffer output_framebuffer;
private:
    LightingShader shader;
    SkyboxShader skybox_shader;
};

class SkyPass : public RenderPass
{
public:
    SkyPass();
    void render(
        const Scene& scene,
        const glm::mat4& view,
        const glm::mat4& projection,
        const Framebuffer& g_buffer
    );
private:
    SkyboxShader shader;
};

class WaterPass : public RenderPass
{
public:
    WaterPass();
    void render(
        const Scene& scene,
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::mat4& light_projection,
        const Texture* cloud_noise,
        const Framebuffer& output_framebuffer
    );

private:
    WaterShader shader;
};

class ShadowPass : public RenderPass
{
public:
    ShadowPass();
    void render(
        const Scene& scene,
        const glm::mat4& light_projection
    );

private:
    ShadowMapShader shader;
};

class BlurPass : public RenderPass
{
public:
    BlurPass(const unsigned int width, const unsigned int height);
    void render(const std::optional<Texture*> texture = {}, std::optional<Framebuffer*> output = {});
    Framebuffer& get_default_input();
    Framebuffer& get_default_output();

private:
    BlurShader shader;
    Framebuffer framebuffers[2];
};

class BloomPass : public RenderPass
{
public:
    BloomPass(const unsigned int width, const unsigned int height);
    void render(const Texture& texture);
    Framebuffer& get_output();

private:
    BloomShader bloom_shader;
    BlurPass blur_pass;
};

class SpritePass : public RenderPass
{
public:
    SpritePass();
    void render(const Scene& scene, const glm::mat4& projection);

private:
    QuadShader shader;
};

class CloudPass : public RenderPass
{
public:
    CloudPass(const unsigned int width, const unsigned int height);
    ~CloudPass();
    void render(
        Scene& scene,
        const glm::mat4& view,
        const glm::mat4& projection,
        const Texture& input_depth
    );
    Framebuffer output_framebuffer;
    Texture* noises[2];

private:
    void init(const float scale);
    CloudShader cloud_shader;
    WorleyShader worley_shader;
    BlurPass blur_pass;
};

class CompositePass : public RenderPass
{
public:
    CompositePass();
    void render(const Texture& one, const Texture& two, const Texture& three);

private:
    CompositeShader shader;
};
