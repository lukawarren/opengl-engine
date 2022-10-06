#include "renderer.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height) :
    window(title, width, height)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);

    diffuse_shader.bind();
}

bool Renderer::update(const std::vector<Entity>& entities, const Camera& camera)
{
    // Update window; poll events
    if (!window.update()) return false;

    // Update uniforms
    diffuse_shader.set_uniform("view", camera.view_matrix());
    diffuse_shader.set_uniform("projection", camera.projection_matrix(
        window.framebuffer_width,
        window.framebuffer_height
    ));

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // TODO: minimise state changes by ordering by mesh, texture, shader, etc.
    for (const auto& entity : entities)
    {
        diffuse_shader.set_uniform("model", entity.transform.matrix());
        entity.texture->bind();

        for (const auto& mesh : entity.meshes)
        {
            mesh->bind();
            mesh->draw();
        }
    }

    return true;
}


