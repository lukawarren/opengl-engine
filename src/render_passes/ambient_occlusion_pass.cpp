#include "render_passes/render_passes.h"
#include <random>

AmbientOcclusionPass::AmbientOcclusionPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    output_framebuffer(width, height, Framebuffer::DepthSettings::NO_DEPTH, false, true),
    blur_pass(width, height)
{
    shader.bind();
    shader.set_uniform("position",  0);
    shader.set_uniform("normal",    1);
    shader.set_uniform("depth",     2);
    shader.set_uniform("noise",     3);

    const auto lerp = [](const float a, const float b, const float f)
    {
        return a + f * (b - a);
    };

    // Generate samples - update value in fragment shader too if changed
    const unsigned int samples = 16;
    std::uniform_real_distribution<float> rand_float(0.0, 1.0);
    std::default_random_engine generator;
    for (unsigned int i = 0; i < samples; ++i)
    {
        // Generate random sample
        glm::vec3 sample(
            rand_float(generator) * 2.0 - 1.0,
            rand_float(generator) * 2.0 - 1.0,
            rand_float(generator)
        );
        sample = glm::normalize(sample);
        sample *= rand_float(generator);

        // Bias towards origin
        float scale = (float)i / (float)samples;
        scale = lerp(0.1f, 1.0f, scale * scale);
        sample *= scale;

        shader.set_uniform("kernel[" + std::to_string(i) + "]", sample);
    }

    // Noise texture
    const unsigned int noise_size = 4;
    std::vector<glm::vec3> noises;
    noises.reserve(noise_size * noise_size);
    for (unsigned int i = 0; i < noise_size * noise_size; i++)
    {
        noises.emplace_back(
            rand_float(generator) * 2.0 - 1.0,
            rand_float(generator) * 2.0 - 1.0,
            0.0f
        );
    }

    noise = new Texture(noise_size, noise_size, GL_RGB16F, GL_RGB, GL_FLOAT, true, (char*)&noises[0]);
    shader.set_uniform("noise_scale", glm::vec2 {
        (float)width / (float)noise_size,
        (float)height / (float)noise_size
    });
}

void AmbientOcclusionPass::render(
    const Texture& positions,
    const Texture& normals,
    const Texture& depth,
    const glm::mat4& view,
    const glm::mat4& projection
)
{
    output_framebuffer.bind();
    shader.bind();

    // ImGui - for a more moderate effect use 0.5f for radius and 1.0f for sharpness
    static float radius = 2.0f;
    static float bias = 0.025f;
    static float sharpness = 2.0f;
    static bool enabled = true;
    ImGui::Begin("SSAO");
    ImGui::SliderFloat("Radius", &radius, 0.0f, 10.0);
    ImGui::SliderFloat("Bias", &bias, 0.0f, 1.0);
    ImGui::SliderFloat("Sharpness", &sharpness, 0.0f, 10.0);
    ImGui::Checkbox("Enabled", &enabled);
    ImGui::End();

    if (!enabled)
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        return;
    }

    // Textures
    positions.bind(0);
    normals.bind(1);
    depth.bind(2);
    noise->bind(3);

    // Uniforms
    shader.set_uniform("view", view);
    shader.set_uniform("projection", projection);
    shader.set_uniform("radius", radius);
    shader.set_uniform("bias", bias);
    shader.set_uniform("sharpness", sharpness);

    quad_mesh->draw();

    // Blur
    blur_pass.render(&output_framebuffer.colour_texture.value(), &output_framebuffer);
}

AmbientOcclusionPass::~AmbientOcclusionPass()
{
    delete noise;
}