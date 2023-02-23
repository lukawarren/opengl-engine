#include "renderer.h"
#include "resources.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height, const float render_scale) :
    window(title, width, height),
    render_scale(render_scale),
    bloom_framebuffer(render_width(), render_height()),
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
    composite_shader.bind();
    composite_shader.set_uniform("image_one", 0);
    composite_shader.set_uniform("image_two", 1);

    // Post processing settings
    bloom_shader.bind();
    bloom_shader.set_uniform("threshold", 1.0f);

    init_resources();
}

bool Renderer::update(const Scene& scene)
{
    // Update window; poll events
    double start = glfwGetTime();
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
    bloom_pass();

    // Display scaled output...
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // ...combining FBOs (with HDR pass)
    composite_shader.bind();
    output_framebuffer.colour_texture->bind();
    bloom_framebuffer.colour_texture->bind(1); // bloom
    quad_mesh->draw();

    glViewport(0, 0, render_width(), render_height());
    glEnable(GL_CULL_FACE);

    // Calculate FPS
    double end = glfwGetTime();
    int fps = int(1.0 / (end - start));

    if (glfwGetTime() - last_fps_report_time >= 1.0)
    {
        std::cout << fps << " fps - " << (end-start) * 1000 << " ms" << std::endl;
        last_fps_report_time = glfwGetTime();
    }

    return true;
}

void Renderer::diffuse_pass(
    const Scene& scene,
    const Camera& camera,
    const unsigned int width,
    const unsigned int height,
    const std::optional<glm::vec4> clip_plane)
{
    const auto view_matrix = camera.view_matrix();
    const auto projection_matrix = camera.projection_matrix(width, height);

    const auto set_common_uniforms = [&](Shader& shader)
    {
        // Update uniforms...
        shader.bind();
        shader.set_uniform("view", view_matrix);
        shader.set_uniform("projection", projection_matrix);
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
        const auto view = glm::mat4(glm::mat3(view_matrix));

        skybox_shader.bind();
        skybox_shader.set_uniform("matrix", projection_matrix * view);
        skybox_shader.set_uniform("tint", scene.skybox_tint);
        (*scene.skybox)->bind();
        cube_mesh->bind();
        cube_mesh->draw();

        glEnable(GL_CULL_FACE);
        glDepthMask(GL_TRUE);
    }

    // Render objects
    scene.sun.shadow_buffer->depth_map->bind(2);
    entities();
    if (scene.chunks.size() > 0) chunks();
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

    // Render chunks - back to normal culling!
    glCullFace(GL_BACK);
    if (scene.chunks.size() > 0) Chunk::texture->bind();
    for (const auto& chunk : scene.chunks)
    {
        shadow_shader.set_uniform("mvp", view_projection * chunk.transform.matrix());
        chunk.mesh->bind();
        chunk.mesh->draw();
    }

    // Restore
    buffer->unbind();
    glViewport(0, 0, render_width(), render_height());
}

void Renderer::bloom_pass()
{
    // Setup framebuffer
    bloom_framebuffer.bind();
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Extract bright parts of image
    output_framebuffer.colour_texture->bind();
    bloom_shader.bind();
    quad_mesh->draw();
    bloom_framebuffer.unbind();

    // Blur
    for (int i = 0; i < 5; ++i)
        blur_pass(*bloom_framebuffer.colour_texture, bloom_framebuffer);
}

void Renderer::blur_pass(const Texture& texture, const Framebuffer& final_framebuffer)
{
    blur_shader.bind();
    quad_mesh->bind();

    // Cheaper to just disable depth tests instead of clearing every time
    glDisable(GL_DEPTH_TEST);

    // Horizontal
    texture.bind();
    blur_framebuffers[0].bind();
    blur_shader.set_uniform("horizontal", true);
    quad_mesh->draw();

    // Vertical
    blur_framebuffers[1].bind();
    blur_framebuffers[0].colour_texture->bind();
    blur_shader.set_uniform("horizontal", false);
    quad_mesh->draw();

    // Horizontal
    blur_framebuffers[0].bind();
    blur_framebuffers[1].colour_texture->bind();
    blur_shader.set_uniform("horizontal", true);
    quad_mesh->draw();

    // Final vertical
    final_framebuffer.bind();
    blur_framebuffers[0].colour_texture->bind();
    blur_shader.set_uniform("horizontal", false);
    quad_mesh->draw();
    final_framebuffer.unbind();

    glEnable(GL_DEPTH_TEST);
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
