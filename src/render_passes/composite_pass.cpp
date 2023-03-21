#include "render_passes/render_passes.h"

CompositePass::CompositePass() : RenderPass()
{
    shader.bind();
    shader.set_uniform("image_one",     0);
    shader.set_uniform("image_two",     1);
    shader.set_uniform("image_three",   2);
}

void CompositePass::render(const Texture& one, const Texture& two, const Texture& three)
{
    shader.bind();
    one.bind();
    two.bind(1);
    three.bind(2);
    quad_mesh->draw();
}
