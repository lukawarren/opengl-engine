#include "render_passes/render_passes.h"

CompositePass::CompositePass() : RenderPass()
{
    shader.bind();
    shader.set_uniform("image_one", 0);
    shader.set_uniform("image_two", 1);
}

void CompositePass::render(const Texture& one, const Texture& two)
{
    shader.bind();
    one.bind();
    two.bind(1);
    quad_mesh->draw();
}
