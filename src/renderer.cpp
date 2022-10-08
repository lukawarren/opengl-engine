#include "renderer.h"
#include "resources.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height) :
    window(title, width, height)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
    init_resources();
}

bool Renderer::update(const Scene& scene)
{
    // Update window; poll events
    if (!window.update()) return false;

    diffuse_pass(scene, scene.camera, window.framebuffer_width, window.framebuffer_height);
    water_pass(scene);

    return true;
}

void Renderer::diffuse_pass(
    const Scene& scene,
    const Camera& camera,
    const unsigned int width,
    const unsigned int height,
    const std::optional<glm::vec4> clip_plane)
{
    // Update uniforms...
    diffuse_shader.bind();
    diffuse_shader.set_uniform("view", camera.view_matrix());
    diffuse_shader.set_uniform("projection", camera.projection_matrix(width, height));

    // ...including clip planes (for planar reflections)
    if (clip_plane.has_value())
        diffuse_shader.set_uniform("clip_plane", clip_plane.value());

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // TODO: sort by least-expensive state-change
    for (const auto& entity : scene.entities)
    {
        diffuse_shader.set_uniform("model", entity.transform.matrix());

        for (const auto& mesh : entity.textured_meshes)
        {
            mesh.texture->bind();
            mesh.mesh->bind();
            mesh.mesh->draw();
        }
    }
}

void Renderer::water_pass(const Scene& scene)
{
    // Reflections: render scene normally, but to water FBOs
    // NOTE: clipping is used so that we can render from behind
    // the plane, but not have "behind geometry" occluding us
    glEnable(GL_CLIP_DISTANCE0);
    for (const auto& water : scene.waters)
    {
        const auto buffer = water.reflection_buffer;
        buffer->bind();

        // Use FBO size for viewport...
        glViewport(0, 0, buffer->width, buffer->height);

        // ...but window size for projection matrix!
        diffuse_pass(
            scene,
            water.reflection_camera(scene.camera),
            window.framebuffer_width,
            window.framebuffer_height,
            glm::vec4 (0, 1, 0, -water.transform.position.y)
        );
    }
    glDisable(GL_CLIP_DISTANCE0);

    // Reset rendering state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
    quad_mesh->bind();

    // Set uniforms
    water_shader.bind();
    water_shader.set_uniform("view", scene.camera.view_matrix());
    water_shader.set_uniform("projection", scene.camera.projection_matrix(
        window.framebuffer_width,
        window.framebuffer_height
    ));

    // Render water
    for (const auto& water : scene.waters)
    {
        water_shader.set_uniform("model", water.transform.matrix());
        water.reflection_buffer->colour_texture.bind();
        quad_mesh->draw();
    }
}

Renderer::~Renderer()
{
    free_resources();
}
