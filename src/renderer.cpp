#include "renderer.h"
#include "resources.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height, const float render_scale) :
    window(title, width, height),
    render_scale(render_scale),
    volumetric_framebuffer(render_width() * VOLUMETRIC_RESOLUTION, render_height() * VOLUMETRIC_RESOLUTION),
    blur_framebuffers {
        Framebuffer(render_width(), render_height()),
        Framebuffer(render_width(), render_height())
    },
    output_framebuffer(render_width(), render_height(), Framebuffer::DepthSettings::ENABLE_DEPTH)
{
    // Setup GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, render_width(), render_height());
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

    // More texture units
    diffuse_shader.bind();
    diffuse_shader.set_uniform("diffuse_map", 0);
    diffuse_shader.set_uniform("normal_map", 1);
    diffuse_shader.set_uniform("shadow_map", 2);
    terrain_shader.bind();
    terrain_shader.set_uniform("diffuse_map", 0);
    terrain_shader.set_uniform("shadow_map", 2);
    volumetrics_shader.bind();
    volumetrics_shader.set_uniform("depth_map", 0);
    volumetrics_shader.set_uniform("shadow_map", 1);
    composite_shader.bind();
    composite_shader.set_uniform("image_one", 0);
    composite_shader.set_uniform("image_two", 1);

    init_resources();
}

bool Renderer::update(const Scene& scene)
{
    // Update window; poll events
    if (!window.update()) return false;

    // Shadows
    if (BAKE_SHADOWMAPS == false || !did_bake_shadows)
    {
        shadow_pass(scene);
        did_bake_shadows = true;
    }

    // Render passes
    output_framebuffer.bind();
    diffuse_pass(scene, scene.camera, render_width(), render_height());
    water_pass(scene);
    output_framebuffer.unbind();

    // Post processing
    glDisable(GL_CULL_FACE);
    quad_mesh->bind();
    volumetrics_pass(scene);

    // Display scaled output...
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // ...combining FBOs
    composite_shader.bind();
    output_framebuffer.colour_texture->bind();
    volumetric_framebuffer.colour_texture->bind(1);
    quad_mesh->draw();

    glViewport(0, 0, render_width(), render_height());
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
        shader.set_uniform("ambient_light", scene.ambient_light);
        shader.set_uniform("light_position", scene.sun.position);
        shader.set_uniform("light_colour", scene.sun.colour);
        shader.set_uniform("lightspace_matrix", scene.sun.get_light_projection_matrix());

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

    const auto terrain = [&]()
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        set_common_uniforms(terrain_shader);

        for (const auto& terrain : scene.terrains)
        {
            terrain_shader.set_uniform("model", terrain.transform.matrix());
            terrain.texture->bind();
            terrain.mesh->bind();
            terrain.mesh->draw();
        }
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    };

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    scene.sun.shadow_buffer->depth_map->bind(2);
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
            render_width(),
            render_height(),
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
            render_width(),
            render_height(),
            glm::vec4 (0, -1, 0, water.transform.position.y+0.1f)
            // Add small offset to fix glitches at edges ^^
        );
    }
    glDisable(GL_CLIP_DISTANCE0);

    // Reset rendering state
    output_framebuffer.bind();
    glViewport(0, 0, render_width(), render_height());
    quad_mesh->bind();

    // Set uniforms
    water_shader.bind();
    water_shader.set_uniform("view", scene.camera.view_matrix());
    water_shader.set_uniform("projection", scene.camera.projection_matrix(
        render_width(),
        render_height()
    ));
    water_shader.set_uniform("camera_position", scene.camera.position);
    water_shader.set_uniform("light_position", scene.sun.position);
    water_shader.set_uniform("light_colour", scene.sun.colour);

    // Render water
    glEnable(GL_BLEND);
    for (const auto& water : scene.waters)
    {
        water_shader.set_uniform("time", water.time);
        water_shader.set_uniform("model", water.transform.matrix());

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

void Renderer::shadow_pass(const Scene& scene)
{
    // "Peter-panning" work-around
    glCullFace(GL_FRONT);

    // Bind framebuffer
    const auto& buffer = scene.sun.shadow_buffer;
    buffer->bind();
    glViewport(0, 0, buffer->width, buffer->height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shadow_shader.bind();

    const glm::mat4 view_projection = scene.sun.get_light_projection_matrix();

    // Render entities
    for (const auto& entity : scene.entities)
    {
        shadow_shader.set_uniform("mvp", view_projection * entity.transform.matrix());

        for (const auto& mesh : entity.textured_meshes)
        {
            mesh.mesh->bind();
            mesh.mesh->draw();
        }
    }

    // Render terrain (disable peter panning as it won't work for completely thin surfaces)
    glCullFace(GL_BACK);
    for (const auto& terrain : scene.terrains)
    {
        shadow_shader.set_uniform("mvp", view_projection * terrain.transform.matrix());
        terrain.mesh->bind();
        terrain.mesh->draw();
    }

    // Restore
    buffer->unbind();
    glViewport(0, 0, render_width(), render_height());
}

void Renderer::volumetrics_pass(const Scene& scene)
{
    auto start = glfwGetTime();

    // Setup framebuffer
    volumetric_framebuffer.bind();
    glViewport(
        0,
        0,
        render_width() * VOLUMETRIC_RESOLUTION,
        render_height() * VOLUMETRIC_RESOLUTION
    );
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Textures
    output_framebuffer.depth_map->bind(0);
    scene.sun.shadow_buffer->depth_map->bind(1);

    // Uniforms
    const auto view = scene.camera.view_matrix();
    const auto projection = scene.camera.projection_matrix(render_width(), render_height());
    volumetrics_shader.bind();
    volumetrics_shader.set_uniform("inverse_view", glm::inverse(view));
    volumetrics_shader.set_uniform("inverse_projection", glm::inverse(projection));
    volumetrics_shader.set_uniform("lightspace", scene.sun.get_light_projection_matrix());
    volumetrics_shader.set_uniform("light_colour", scene.sun.colour);
    quad_mesh->draw();

    // Restore
    volumetric_framebuffer.unbind();
    glViewport(0, 0, render_width(), render_height());

    // Blur
    blur_pass(volumetric_framebuffer.colour_texture->texture_id);

    glFinish();
    auto end = glfwGetTime();
    std::cout << (end - start) * 1000 << std::endl;
}

void Renderer::blur_pass(unsigned int texture_id)
{

}

unsigned int Renderer::render_width() const
{
    return window.framebuffer_width * render_scale;
}

unsigned int Renderer::render_height() const
{
    return window.framebuffer_height * render_scale;
}

Renderer::~Renderer()
{
    free_resources();
}
