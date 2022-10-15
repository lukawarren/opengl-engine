#include "renderer.h"
#include "resources.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height, const float render_scale) :
    window(title, width, height),
    output_framebuffer(window.framebuffer_width * render_scale, window.framebuffer_height * render_scale)
{
    // Setup GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, output_width(), output_height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // For water soft edges
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Connect texture units
    water_shader.bind();
    water_shader.set_uniform("reflection_texture", 0);
    water_shader.set_uniform("refraction_texture", 1);
    water_shader.set_uniform("depth_map", 2);
    water_shader.set_uniform("distortion_map", 3);
    water_shader.set_uniform("normal_map", 4);

    // Permanent uniforms
    water_shader.set_uniform("z_near", z_near);
    water_shader.set_uniform("z_far", z_far);
    water_shader.unbind();

    init_resources();
    this->render_scale = render_scale;
}

bool Renderer::update(const Scene& scene)
{
    // Update window; poll events
    if (!window.update()) return false;

    // Render passes
    output_framebuffer.bind();
    diffuse_pass(scene, scene.camera, output_width(), output_height());
    water_pass(scene);
    output_framebuffer.unbind();

    // Display scaled output
    glDisable(GL_CULL_FACE);
        glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
            output_framebuffer.colour_texture.bind();
            quad_shader.bind();
            quad_mesh->bind();
            quad_mesh->draw();
        glViewport(0, 0, output_width(), output_height());
    glEnable(GL_CULL_FACE);

    return true;
}

void Renderer::diffuse_pass(
    const Scene& scene,
    const Camera& camera,
    const unsigned int width,
    const unsigned int height,
    const std::optional<glm::vec4> clip_plane)
{
    const auto set_common_uniforms = [&](Shader& shader)
    {
        // Update uniforms...
        shader.bind();
        shader.set_uniform("view", camera.view_matrix());
        shader.set_uniform("projection", camera.projection_matrix(width, height));

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
                mesh.texture->bind();
                mesh.mesh->bind();
                mesh.mesh->draw();
            }
        }
    };

    const auto terrain = [&]()
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        set_common_uniforms(terrain_shader);

        for (const auto& terrain : scene.terrains)
        {
            terrain_shader.set_uniform("model", terrain.transform.matrix());
            terrain.mesh->bind();
            terrain.mesh->draw();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    };

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    entities();
    terrain();
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
            output_width(),
            output_height(),
            glm::vec4(0, 1, 0, -water.transform.position.y+0.1f)
            // Add small offset to fix glitches at edges ^^
        );
    }

    // Refractions: much the same!
    for (const auto& water : scene.waters)
    {
        const auto buffer = water.refraction_buffer;
        buffer->bind();

        // Use FBO size for viewport...
        glViewport(0, 0, buffer->width, buffer->height);

        // ...but window size for projection matrix!
        diffuse_pass(
            scene,
            scene.camera,
            output_width(),
            output_height(),
            glm::vec4 (0, -1, 0, water.transform.position.y+0.1f)
            // Add small offset to fix glitches at edges ^^
        );
    }
    glDisable(GL_CLIP_DISTANCE0);

    // Reset rendering state
    output_framebuffer.bind();
    glViewport(0, 0, output_width(), output_height());
    quad_mesh->bind();

    // Set uniforms
    water_shader.bind();
    water_shader.set_uniform("view", scene.camera.view_matrix());
    water_shader.set_uniform("projection", scene.camera.projection_matrix(
        output_width(),
        output_height()
    ));
    water_shader.set_uniform("camera_position", scene.camera.position);

    // Render water
    glEnable(GL_BLEND);
    for (const auto& water : scene.waters)
    {
        water_shader.set_uniform("time", water.time);
        water_shader.set_uniform("model", water.transform.matrix());

        // Texture units
        water.reflection_buffer->colour_texture.bind(0);
        water.refraction_buffer->colour_texture.bind(1);
        water.refraction_buffer->depth_map->bind(2);
        water.distortion_map->bind(3);
        water.normal_map->bind(4);

        quad_mesh->draw();
    }
    glDisable(GL_BLEND);
}

unsigned int Renderer::output_width() const
{
    return window.framebuffer_width * render_scale;
}

unsigned int Renderer::output_height() const
{
    return window.framebuffer_height * render_scale;
}

Renderer::~Renderer()
{
    free_resources();
}
