#pragma once
#include "../shader.h"
#include "../scene.h"

class RenderPass
{
public:
    RenderPass() {}
    virtual ~RenderPass() {};
};

class DiffusePass : public RenderPass
{
public:
    DiffusePass();
    void render(
        const Scene& scene,
        const glm::mat4& view,
        const glm::mat4& projection,
        const glm::mat4& light_projection,
        const Texture* cloud_noise,
        const std::optional<glm::vec4> clip_plane = {}
    );

private:
    DiffuseShader diffuse_shader;
    SkyboxShader skybox_shader;
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
        const Framebuffer& output_framebuffer,
        DiffusePass& diffuse
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

class BloomPass : public RenderPass
{
public:
    BloomPass(const unsigned int width, const unsigned int height);
    void render(const Texture& texture);
    Framebuffer output_framebuffer;

private:
    void blur(const Texture& texture);
    BlurShader blur_shader;
    BloomShader bloom_shader;
    Framebuffer blur_framebuffers[2];
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
        const Framebuffer& input_framebuffer
    );
    Framebuffer output_framebuffer;
    Texture* noises[2];

private:
    void init(const float scale);
    CloudShader cloud_shader;
    WorleyShader worley_shader;
};

class CompositePass : public RenderPass
{
public:
    CompositePass();
    void render(const Texture& one, const Texture& two);

private:
    CompositeShader shader;
};
