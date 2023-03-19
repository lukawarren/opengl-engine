#include "renderer.h"
#include "resources.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

Renderer::Renderer(const std::string& title, const int width, const int height, const float render_scale) :
    window(title, width, height),
    render_scale(render_scale),
    output_framebuffer(render_width(), render_height(), Framebuffer::DepthSettings::ENABLE_DEPTH),
    bloom_pass(render_width(), render_height()),
    cloud_pass(render_width(), render_height())
{
    // Setup GL state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glViewport(0, 0, render_width(), render_height());
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // For water soft edges
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Init ImGui
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window.window, true);
    ImGui_ImplOpenGL3_Init("#version 150");

    init_resources();
}

bool Renderer::update(Scene& scene)
{
    // Update window; poll events
    double start = glfwGetTime();
    if (!window.update()) return false;

    // ImGui
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Compute matrices
    const auto view = scene.camera.view_matrix();
    const auto projection = scene.camera.projection_matrix(render_width(), render_height());
    const auto light_projection = scene.sun.get_light_projection_matrix(scene.camera, render_width(), render_height());

    // Shadows
    if (BAKE_SHADOWMAPS == false || !did_bake_shadows)
    {
        shadow_pass.render(scene, light_projection);
        did_bake_shadows = true;
    }

    // "Normal" render passes
    output_framebuffer.bind();
    diffuse_pass.render(
        scene,
        view,
        projection,
        light_projection,
        cloud_pass.noises[0]
    );
    water_pass.render(
        scene,
        view,
        projection,
        light_projection,
        cloud_pass.noises[0],
        output_framebuffer,
        diffuse_pass
    );

    // Clouds (whose framebuffer now becomes the main output)
    cloud_pass.render(
        scene,
        view,
        projection,
        output_framebuffer
    );

    // Post processing - all passes need the quad mesh bound
    glDisable(GL_CULL_FACE);
    quad_mesh->bind();
    bloom_pass.render(cloud_pass.output_framebuffer.colour_texture.value());

    // Display scaled output...
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, window.framebuffer_width, window.framebuffer_height);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // ...combining FBOs (with HDR pass)
    composite_pass.render(
        *cloud_pass.output_framebuffer.colour_texture,
        *bloom_pass.output_framebuffer.colour_texture
    );

    // Sprites
    sprite_pass.render(scene, projection);

    // ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

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
