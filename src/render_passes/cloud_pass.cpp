#include "render_passes/render_passes.h"

CloudPass::CloudPass(const unsigned int width, const unsigned int height) :
    RenderPass(),
    output_framebuffer(width, height)
{
    cloud_shader.bind();
    cloud_shader.bind();
    cloud_shader.set_uniform("noise_map",       0);
    cloud_shader.set_uniform("detail_map",      1);
    cloud_shader.set_uniform("depth_map",       2);
    cloud_shader.set_uniform("framebuffer",     3);
    cloud_shader.set_uniform("z_near",          z_near);
    cloud_shader.set_uniform("z_far",           z_far);

    init(default_texture_scale);
}

void CloudPass::init(const float scale)
{
    // Create texture for compute shader
    int width = 64;
    int height = 64;
    int depth = 64;
    noises[0] = new Texture(width, height, GL_R32F, GL_RED, GL_FLOAT, false, nullptr, depth);
    noises[0]->bind_image(GL_R32F, GL_READ_WRITE);

    // Generate worley noise and wait
    worley_shader.bind();
    worley_shader.set_uniform("output_size", glm::vec3 { width, height, depth });
    worley_shader.set_uniform("scale", scale);
    worley_shader.set_uniform("just_perlin", false);
    glDispatchCompute(width, height, depth);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Generate high-res simplex noise
    width = 256;
    height = 256;
    depth = 256;
    noises[1] = new Texture(width, height, GL_R32F, GL_RED, GL_FLOAT, false, nullptr, depth);
    noises[1]->bind_image(GL_R32F, GL_READ_WRITE);

    // Same compute shader; different params
    worley_shader.set_uniform("output_size", glm::vec3 { width, height, depth });
    worley_shader.set_uniform("scale", scale * 5.0f);
    worley_shader.set_uniform("just_perlin", true);
    glDispatchCompute(width, height, depth);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void CloudPass::render(
    Scene& scene,
    const glm::mat4& view,
    const glm::mat4& projection,
    const Texture& input_colour,
    const Texture& input_depth
)
{
    // Bounds
    auto& cloud = scene.cloud_settings;
    glm::vec3 min_bounds = glm::vec3(
        scene.camera.position.x - cloud.size,
        cloud.height_min,
        scene.camera.position.z - cloud.size
    );
    glm::vec3 max_bounds = glm::vec3(
        scene.camera.position.x + cloud.size,
        cloud.height_max,
        scene.camera.position.z + cloud.size
    );

    const glm::mat4 view_projection = projection * view;

    // GUI
    if (cloud.draw_debug_gui())
    {
        delete noises[0];
        delete noises[1];
        init(cloud.texture_scale);
    }

    cloud_shader.bind();

    // Scene info
    cloud_shader.set_uniform("mvp", glm::mat4(1.0f));
    cloud_shader.set_uniform("inverse_view_projection", glm::inverse(view_projection));
    cloud_shader.set_uniform("camera_position", scene.camera.position);
    cloud_shader.set_uniform("bounds_min", min_bounds);
    cloud_shader.set_uniform("bounds_max", max_bounds);
    cloud_shader.set_uniform("screen_size", glm::vec2 {
        output_framebuffer.width,
        output_framebuffer.height
    });
    cloud_shader.set_uniform("light_colour", scene.sun.colour);

    // Noise
    cloud_shader.set_uniform("offset", cloud.time);

    // Scattering settings
    cloud_shader.set_uniform("scale", cloud.scale);
    cloud_shader.set_uniform("detail_scale", cloud.detail_scale);
    cloud_shader.set_uniform("density", cloud.density);
    cloud_shader.set_uniform("threshold", cloud.threshold);
    cloud_shader.set_uniform("brightness", cloud.brightness);
    cloud_shader.set_uniform("steps", cloud.steps);

    // Render
    output_framebuffer.bind();
    glEnable(GL_BLEND);
    glClear(GL_COLOR_BUFFER_BIT);
    noises[0]->bind(0);
    noises[1]->bind(1);
    input_depth.bind(2);
    input_colour.bind(3);
    quad_mesh->bind();
    quad_mesh->draw();
    glDisable(GL_BLEND);
}

CloudPass::~CloudPass()
{
    delete noises[0];
    delete noises[1];
}
