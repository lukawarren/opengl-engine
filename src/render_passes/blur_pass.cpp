#include "render_passes/render_passes.h"

BlurPass::BlurPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    framebuffers {
        Framebuffer(width, height),
        Framebuffer(width, height)
    }
{
    framebuffers[0].colour_texture->clamp(glm::vec4(0.0f));
    framebuffers[1].colour_texture->clamp(glm::vec4(0.0f));
}

void BlurPass::render(const std::optional<Texture*> texture, std::optional<Framebuffer*> output)
{
    shader.bind();
    quad_mesh->bind();

    // Horizontal
    if (texture.has_value()) (*texture)->bind();
    else get_default_input().colour_texture->bind();
    framebuffers[0].bind();
    shader.set_uniform("horizontal", true);
    quad_mesh->draw();

    // Vertical
    framebuffers[1].bind();
    framebuffers[0].colour_texture->bind();
    shader.set_uniform("horizontal", false);
    quad_mesh->draw();

    // Horizontal
    framebuffers[0].bind();
    framebuffers[1].colour_texture->bind();
    shader.set_uniform("horizontal", true);
    quad_mesh->draw();

    // Final vertical
    if (output.has_value()) (*output)->bind();
    else framebuffers[1].bind();
    framebuffers[0].colour_texture->bind();
    shader.set_uniform("horizontal", false);
    quad_mesh->draw();
}

Framebuffer& BlurPass::get_default_input()
{
    return framebuffers[1];
}

Framebuffer& BlurPass::get_default_output()
{
    return framebuffers[1];
}