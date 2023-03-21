#include "render_passes/render_passes.h"

LightingPass::LightingPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    output_framebuffer(width, height)
{
    shader.bind();
    shader.set_uniform("g_albedo",   0);
    shader.set_uniform("g_normal",   1);
    shader.set_uniform("g_position", 2);
    shader.set_uniform("cloud_map",  3);
    shader.set_uniform("shadow_map", 4);
}

void LightingPass::render(
    const Scene& scene,
    const glm::mat4& light_projection,
    const Texture* cloud_noise,
    const Texture& shadow_map,
    const Framebuffer& g_buffer
)
{
    output_framebuffer.bind();
    glClear(GL_COLOR_BUFFER_BIT);

    // Uniforms
    shader.bind();
    shader.set_uniform("ambient_light", scene.ambient_light);
    shader.set_uniform("light_position", scene.sun.position);
    shader.set_uniform("light_colour", scene.sun.colour);
    shader.set_uniform("lightspace", light_projection);
    shader.set_uniform("cloud_scale", scene.cloud_settings.scale * 4.0f);
    shader.set_uniform("cloud_offset", scene.cloud_settings.time);

    // Textures
    g_buffer.colour_texture->bind(0);
    g_buffer.normal_texture->bind(1);
    g_buffer.position_texture->bind(2);
    cloud_noise->bind(3);
    shadow_map.bind(4);

    quad_mesh->bind();
    quad_mesh->draw();
}