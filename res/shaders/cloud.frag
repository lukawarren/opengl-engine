#version 330 core
#define M_PI 3.14159265359

// Scene info
uniform mat4 inverse_view_projection;
uniform vec3 camera_position;
uniform vec3 bounds_min;
uniform vec3 bounds_max;
uniform vec2 screen_size;
uniform float z_near;
uniform float z_far;
uniform vec3 light_colour;

// Noise
uniform sampler3D noise_map;
uniform sampler3D detail_map;
uniform sampler2D depth_map;
uniform sampler2D framebuffer;
uniform float offset;

// Scattering settings
uniform float scale = 0.2;
uniform float detail_scale = 1.2;
uniform float density = 10;
uniform float threshold = 0.75;
uniform float brightness = 8.0;

// Quality settings
uniform int steps = 256;

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
    // Sample main texture
    vec3 texture_pos = (position + offset) * 0.01 * scale;
    float sample = texture(noise_map, texture_pos).r;
    float d = max(0, sample - threshold) * density;

    // Fade at edges
    const float fade_distance = 50;
    float edge_distance_x = min(fade_distance, min(position.x - bounds_min.x, bounds_max.x - position.x));
    float edge_distance_y = min(fade_distance, min(position.y - bounds_min.y, bounds_max.y - position.y));
    float edge_distance_z = min(fade_distance, min(position.z - bounds_min.z, bounds_max.z - position.z));
    float weight = min(edge_distance_x, min(edge_distance_y, edge_distance_z)) / fade_distance;
    d *= weight;

    // Add detailed noise
    float detail = texture(detail_map, (texture_pos + offset * 0.001) * detail_scale).r;
    d -= detail * (1-d);

    return min(max(d, 0), 1);
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
    float light_energy = 0;

    while (distance_travelled < max_distance)
    {
        vec3 ray_position = ray_origin + ray_direction * (distance_to + distance_travelled);

        // Sample density
        float density = get_density(ray_position);
        total_density += density * step_size;
        distance_travelled += step_size;

        // Sample light
        light_energy += exp(-density);
    }

    float transmittance = exp(-total_density);

    // Silver lining
    float silver_lining = transmittance;
    light_energy += silver_lining * 100;
    light_energy /= steps;

    vec4 colour = vec4(1, 1, 1, 1) * light_energy * 0.1 * brightness * vec4(light_colour, 1.0);

    // Blend with scene
    vec4 original_colour = texture(framebuffer, screen_space);
    frag_colour = original_colour * transmittance;
    frag_colour = mix(colour, original_colour, transmittance);
}
