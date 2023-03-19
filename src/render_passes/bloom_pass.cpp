#include "render_passes/render_passes.h"

BloomPass::BloomPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    output_framebuffer(width, height),
    blur_framebuffers {
        Framebuffer(width, height),
        Framebuffer(width, height)
    }
{
    bloom_shader.bind();
    bloom_shader.set_uniform("threshold", 1.0f);
}

void BloomPass::render(const Texture& texture)
{
    // Setup framebuffer
    output_framebuffer.bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Extract bright parts of image
    bloom_shader.bind();
    texture.bind();
    quad_mesh->draw();

    // Blur
    for (int i = 0; i < 5; ++i)
        blur(*output_framebuffer.colour_texture);
}

void BloomPass::blur(const Texture& texture)
{
    blur_shader.bind();
    quad_mesh->bind();

    // Cheaper to just disable depth tests instead of clearing every time
    glDisable(GL_DEPTH_TEST);

    // Horizontal
    texture.bind();
    blur_framebuffers[0].bind();
    blur_shader.set_uniform("horizontal", true);
    quad_mesh->draw();

    // Vertical
    blur_framebuffers[1].bind();
    blur_framebuffers[0].colour_texture->bind();
    blur_shader.set_uniform("horizontal", false);
    quad_mesh->draw();

    // Horizontal
    blur_framebuffers[0].bind();
    blur_framebuffers[1].colour_texture->bind();
    blur_shader.set_uniform("horizontal", true);
    quad_mesh->draw();

    // Final vertical
    output_framebuffer.bind();
    blur_framebuffers[0].colour_texture->bind();
    blur_shader.set_uniform("horizontal", false);
    quad_mesh->draw();

    glEnable(GL_DEPTH_TEST);
}