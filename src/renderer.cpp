#include "renderer.h"
#include "resources.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height) :
    window(title, width, height),
    framebuffer(width, height)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
    init_resources();
}

bool Renderer::update(const std::vector<Entity>& entities,
    const std::vector<Water>& waters, const Camera& camera)
{
    // Update window; poll events
    if (!window.update()) return false;

    // Update uniforms
    for (auto shader : shaders)
    {
        shader->bind();
        shader->set_uniform("view", camera.view_matrix());
        shader->set_uniform("projection", camera.projection_matrix(
            window.framebuffer_width,
            window.framebuffer_height
        ));
    }

    framebuffer.bind();
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    //TODO: minimise state changes by ordering by mesh, texture, shader, etc.
    diffuse_shader.bind();
    for (const auto& entity : entities)
    {
        diffuse_shader.set_uniform("model", entity.transform.matrix());

        for (const auto& mesh : entity.textured_meshes)
        {
            mesh.texture->bind();
            mesh.mesh->bind();
            mesh.mesh->draw();
        }
    }
    framebuffer.unbind();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    water_shader.bind();
    quad_mesh->bind();
    framebuffer.colour_texture.bind();
    for (const auto& water : waters)
    {
        water_shader.set_uniform("model", water.transform.matrix());
        quad_mesh->draw();
    }

    return true;
}

Renderer::~Renderer()
{
    free_resources();
}
