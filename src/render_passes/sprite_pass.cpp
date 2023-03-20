#include "render_passes/render_passes.h"

SpritePass::SpritePass() : RenderPass() {}

void SpritePass::render(const Scene& scene, const glm::mat4& projection)
{
    glEnable(GL_BLEND);
    shader.bind();

    // Disregard translation
    const glm::mat4& raw_projection = glm::mat4(glm::mat3(projection));

    for (const auto& sprite : scene.sprites)
    {
        // Quad mesh already bound
        sprite.texture->bind();
        shader.set_uniform("matrix", raw_projection * sprite.transform.matrix());
        quad_mesh->draw();
    }

    glDisable(GL_BLEND);
}