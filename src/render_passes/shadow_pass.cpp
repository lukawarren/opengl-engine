#include "render_passes/render_passes.h"

ShadowPass::ShadowPass() : RenderPass() {}

void ShadowPass::render(
    const Scene& scene,
    const glm::mat4& light_projection
)
{
    // "Peter-panning" work-around
    glCullFace(GL_FRONT);

    // Bind state
    scene.sun.shadow_buffer->bind();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.bind();

    // Render entities
    for (const auto& entity : scene.entities)
    {
        shader.set_uniform("mvp", light_projection * entity.transform.matrix());

        for (const auto& mesh : entity.textured_meshes)
        {
            mesh.mesh->bind();
            mesh.mesh->draw();
        }
    }

    // Render chunks - back to normal culling!
    glCullFace(GL_BACK);
    if (scene.chunks.size() > 0)
    {
        Chunk::texture->bind();
        for (const auto& chunk : scene.chunks)
        {
            shader.set_uniform("mvp", light_projection * chunk.transform.matrix());
            chunk.mesh->bind();
            chunk.mesh->draw();
        }
    }
}
