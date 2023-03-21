#include "render_passes/render_passes.h"

BloomPass::BloomPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    blur_pass(width, height)
{
    bloom_shader.bind();
    bloom_shader.set_uniform("threshold", 0.7f);
}

void BloomPass::render(const Texture& texture)
{
    // Use blur pass's framebuffer because it's the same size as ours
    // and won't break anything :)
    blur_pass.get_default_input().bind();
    glClear(GL_COLOR_BUFFER_BIT);

    // Extract bright parts of image
    bloom_shader.bind();
    texture.bind();
    quad_mesh->draw();

    // Blur
    blur_pass.render();
}

Framebuffer& BloomPass::get_output()
{
    return blur_pass.get_default_output();
}
