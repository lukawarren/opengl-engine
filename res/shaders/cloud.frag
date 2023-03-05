#version 330 core

// Scene info
uniform mat4 inverse_view_projection;
uniform vec3 camera_position;
uniform vec3 bounds_min;
uniform vec3 bounds_max;
uniform vec2 screen_size;
uniform float z_near;
uniform float z_far;

// Noise
uniform sampler3D noise_map;
uniform sampler2D depth_map;
uniform sampler2D framebuffer;

// Scattering settings
uniform float scale = 0.05;
uniform float density = 10;
uniform float threshold = 0.1;
const int steps = 128;

layout (location = 0) out vec4 frag_colour;

vec2 get_ray_distance_to_box(vec3 position, vec3 direction)
{
    vec3 t0 = (bounds_min - position) / direction;
    vec3 t1 = (bounds_max - position) / direction;
    vec3 t_min = min(t0, t1);
    vec3 t_max = max(t0, t1);

    float distance_a = max(max(t_min.x, t_min.y), t_min.z);
    float distance_b = min(t_max.x, min(t_max.y, t_max.z));

    float distance_to = max(0, distance_a);
    float distance_inside = max(0, distance_b - distance_to);
    return vec2(distance_to, distance_inside);
}

float get_density(vec3 position)
{
    vec3 texture_pos = position * scale;
    float sample = texture(noise_map, texture_pos).r;
    return max(0, sample - threshold) * density;
}

float linearise_depth(float d)
{
    return z_near * z_far / (z_far + d * (z_near - z_far));
}

void main()
{
    // Figure out ray direction
    vec2 screen_space = gl_FragCoord.xy / screen_size;
    vec4 frag_position = vec4(screen_space * 2.0 - 1.0, 1.0, 1.0);
    vec4 frag_direction = inverse_view_projection * frag_position;

    vec3 ray_origin = camera_position;
    vec3 ray_direction = normalize(frag_direction.xyz);

    // Sample depth map
    float depth_sample = texture(depth_map, screen_space.xy).r;
    float depth = linearise_depth(depth_sample);

    // Signed distance field calculation
    vec2 ray_info = get_ray_distance_to_box(ray_origin, ray_direction);
    float distance_to = ray_info.x;
    float distance_inside = ray_info.y;

    float distance_travelled = 0;
    float step_size = distance_inside / steps;
    float max_distance = min(depth - distance_to, distance_inside);

    // Perform ray march along intersection to get average density
    float total_density = 0;
    while (distance_travelled < max_distance)
    {
        vec3 ray_position = ray_origin + ray_direction * (distance_to + distance_travelled);
        total_density += get_density(ray_position) * step_size;
        distance_travelled += step_size;
    }
    float transmittance = exp(-total_density);

    // Blend with scene
    vec4 original_colour = texture(framebuffer, screen_space);
    frag_colour = original_colour * transmittance;
}