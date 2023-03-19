#include "render_passes/render_passes.h"

DiffusePass::DiffusePass() : RenderPass()
{
    diffuse_shader.bind();
    diffuse_shader.set_uniform("diffuse_map",   0);
    diffuse_shader.set_uniform("normal_map",    1);
    diffuse_shader.set_uniform("shadow_map",    2);
    diffuse_shader.set_uniform("cloud_map",     3);
}

void DiffusePass::render(
    const Scene& scene,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::mat4& light_projection,
    const Texture* cloud_noise,
    const std::optional<glm::vec4> clip_plane
)
{
    const auto set_common_uniforms = [&](Shader& shader)
    {
        // Update uniforms...
        shader.bind();
        shader.set_uniform("view", view);
        shader.set_uniform("projection", projection);
        shader.set_uniform("lightspace_matrix", light_projection);

        shader.set_uniform("ambient_light", scene.ambient_light);
        shader.set_uniform("light_position", scene.sun.position);
        shader.set_uniform("light_colour", scene.sun.colour);

        shader.set_uniform("cloud_scale", scene.cloud_settings.scale * 4.0f);
        shader.set_uniform("cloud_offset", scene.cloud_settings.time);

        // ...including clip planes (for planar reflections)
        if (clip_plane.has_value())
            shader.set_uniform("clip_plane", clip_plane.value());
    };

    const auto entities = [&]()
    {
        set_common_uniforms(diffuse_shader);

        // TODO: sort by least-expensive state-change
        for (const auto& entity : scene.entities)
        {
            diffuse_shader.set_uniform("model", entity.transform.matrix());

            for (const auto& mesh : entity.textured_meshes)
            {
                mesh.material.diffuse_texture->bind();

                // Normal mapping
                const auto& normal_map = mesh.material.normal_map;
                diffuse_shader.set_uniform("has_normal_map", normal_map.has_value());
                if (normal_map.has_value()) normal_map.value()->bind(1);

                mesh.mesh->bind();
                mesh.mesh->draw();
            }
        }
    };

    const auto chunks = [&]()
    {
        set_common_uniforms(diffuse_shader);
        Chunk::texture->bind();

        // TODO: sort by least-expensive state-change
        for (const auto& chunk : scene.chunks)
        {
            diffuse_shader.set_uniform("model", chunk.transform.matrix());
            chunk.mesh->bind();
            chunk.mesh->draw();
        }
    };

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Render skybox - disable culling because cube mesh has incorrect winding (TODO: fix)
    if (scene.skybox.has_value())
    {
        glDepthMask(GL_FALSE);
        glDisable(GL_CULL_FACE);

        // Remove translation element from view matrix
        const auto view_matrix = glm::mat4(glm::mat3(view));

        skybox_shader.bind();
        skybox_shader.set_uniform("matrix", projection * view_matrix);
        skybox_shader.set_uniform("tint", scene.skybox_tint);
        (*scene.skybox)->bind();
        cube_mesh->bind();
        cube_mesh->draw();

        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
    }

    // Common textures
    scene.sun.shadow_buffer->depth_map->bind(2);
    cloud_noise->bind(3);

    // Render objects
    entities();
    if (scene.chunks.size() > 0) chunks();
}