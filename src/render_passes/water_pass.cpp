#include "render_passes/render_passes.h"

WaterPass::WaterPass() : RenderPass()
{
    shader.bind();
    shader.set_uniform("reflection_texture",  0);
    shader.set_uniform("refraction_texture",  1);
    shader.set_uniform("depth_map",           2);
    shader.set_uniform("distortion_map",      3);
    shader.set_uniform("normal_map",          4);
    shader.set_uniform("z_near",              z_near);
    shader.set_uniform("z_far",               z_far);
}

void WaterPass::render(
    const Scene& scene,
    const glm::mat4& view,
    const glm::mat4& projection,
    const glm::mat4& light_projection,
    const Texture* cloud_noise,
    const Framebuffer& output_framebuffer,
    DiffusePass& diffuse
)
{
    // NOTE: clipping is used so that we can render from behind
    // the plane, but not have "behind geometry" occluding us
    glEnable(GL_CLIP_DISTANCE0);
    for (const auto& water : scene.waters)
    {
        // Reflections: render scene normally, but to water FBOs
        auto buffer = water.reflection_buffer;
        buffer->bind();

        // Use new view matrix, but old (normal) projection matrix
        const glm::mat4 reflection_view = water.reflection_camera(scene.camera).view_matrix();
        diffuse.render(
            scene,
            reflection_view,
            projection,
            light_projection,
            cloud_noise,
            glm::vec4(0, 1, 0, -water.transform.position.y+0.1f)
            // Add small offset to fix glitches at edges ^^
        );

        // Refractions: much the same (but with default view matrix)
        buffer = water.refraction_buffer;
        buffer->bind();
        diffuse.render(
            scene,
            view,
            projection,
            light_projection,
            cloud_noise,
            glm::vec4(0, -1, 0, water.transform.position.y+0.1f)
            // Add small offset to fix glitches at edges ^^
        );
    }
    glDisable(GL_CLIP_DISTANCE0);

    // Reset rendering state
    output_framebuffer.bind();
    quad_mesh->bind();

    // Set uniforms
    shader.bind();
    shader.set_uniform("view", view);
    shader.set_uniform("projection", projection);
    shader.set_uniform("camera_position", scene.camera.position);
    shader.set_uniform("light_position", scene.sun.position);
    shader.set_uniform("light_colour", scene.sun.colour);

    // Render water
    glEnable(GL_BLEND);
    for (const auto& water : scene.waters)
    {
        shader.set_uniform("time", water.time);
        shader.set_uniform("model", water.transform.matrix());

        // Texture units
        water.reflection_buffer->colour_texture->bind(0);
        water.refraction_buffer->colour_texture->bind(1);
        water.refraction_buffer->depth_map->bind(2);
        water.distortion_map->bind(3);
        water.normal_map->bind(4);

        quad_mesh->draw();
    }
    glDisable(GL_BLEND);
}