#include "render_passes/render_passes.h"

GBufferPass::GBufferPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    g_buffer(width, height, Framebuffer::DepthSettings::ENABLE_DEPTH, true)
{
    shader.bind();
    shader.set_uniform("diffuse_map", 0);
    shader.set_uniform("normal_map",  1);
}

void GBufferPass::render(
    const Scene& scene,
    const glm::mat4& view,
    const glm::mat4& projection,
    const std::optional<glm::vec4> clip_plane
)
{
    shader.bind();
    g_buffer.bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Common uniforms
    shader.set_uniform("view_projection", projection * view);
    if (clip_plane.has_value())
        shader.set_uniform("clip_plane", clip_plane.value());

    // Entities - TODO: sort by least expensive state change
    for (const auto& entity : scene.entities)
    {
        shader.set_uniform("model", entity.transform.matrix());

        for (const auto& mesh : entity.textured_meshes)
        {
            mesh.material.diffuse_texture->bind();

            // Normal mapping
            const auto& normal_map = mesh.material.normal_map;
            shader.set_uniform("has_normal_map", normal_map.has_value());
            if (normal_map.has_value()) normal_map.value()->bind(1);

            mesh.mesh->bind();
            mesh.mesh->draw();
        }
    }

    // Chunks
    if (scene.chunks.size() > 0)
    {
        Chunk::texture->bind();
        shader.set_uniform("has_normal_map", false);

        for (const auto& chunk : scene.chunks)
        {
            shader.set_uniform("model", chunk.transform.matrix());
            chunk.mesh->bind();
            chunk.mesh->draw();
        }
    }
}