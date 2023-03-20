#include "render_passes/render_passes.h"

SkyPass::SkyPass() : RenderPass()
{
    shader.bind();
    shader.set_uniform("skybox", 0);
    shader.set_uniform("depth",  1);
}

void SkyPass::render(
    const Scene& scene,
    const glm::mat4& view,
    const glm::mat4& projection,
    const Framebuffer& g_buffer
)
{
    if (!scene.skybox.has_value()) return;

    // Remove translation element from view matrix
    const auto view_matrix = glm::mat4(glm::mat3(view));

    // Uniforms
    shader.bind();
    shader.set_uniform("matrix", projection * view_matrix);
    shader.set_uniform("screen_size", glm::vec2 { g_buffer.width, g_buffer.height });
    shader.set_uniform("tint", scene.skybox_tint);

    // Textures
    (*scene.skybox)->bind();
    g_buffer.depth_map->bind(1);

    // Draw
    cube_mesh->bind();
    cube_mesh->draw();
}