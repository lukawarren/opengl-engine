#version 330 core

in vec2 out_texture_coord;

const int SAMPLES = 15;
const float intensity = 12.0;
const float scattering = 0.5;
const float pi = 3.14159265358979;

// 4x4 bayer matrix to add noise (lets us get away with less samples)
const float dither_pattern[16] = float[]
(
    0.0, 0.5, 0.125, 0.625,
    0.75, 0.22, 0.875, 0.375,
    0.1875, 0.6875, 0.0625, 0.5625,
    0.9375, 0.4375, 0.8125, 0.3125
);

uniform mat4 inverse_view;
uniform mat4 inverse_projection;
uniform mat4 lightspace;
uniform vec3 light_position;
uniform vec3 light_colour;
uniform sampler2D depth_map;
uniform sampler2D shadow_map;

layout (location = 0) out vec4 frag_colour;

vec3 world_pos_from_depth(float depth, vec2 uv)
{
    float z = depth * 2.0 - 1.0;
    vec4 clip_space = vec4(uv * 2.0 - 1.0, z, 1.0);
    vec4 view_space = inverse_projection * clip_space;
    view_space /= view_space.w;
    vec4 world_space = inverse_view * view_space;
    return world_space.xyz;
}

float get_shadow(vec4 lightspace_position)
{
    // Perspective divide
    vec3 proj_coords = lightspace_position.xyz / lightspace_position.w;

    // Transform to [0, 1] range
    proj_coords = proj_coords * 0.5 + 0.5;

    // If outside of shadow map, ditch
    if(proj_coords.z > 1.0)
        return 0;

    // Sample shadowmap
    float current_depth = proj_coords.z;
    float closest_depth = texture(shadow_map, proj_coords.xy).r;
    return current_depth > closest_depth ? 0.0 : 1.0;
}

// "Mie scattering" based on Henyey-Greenstein phase function
// https://www.alexandre-pestana.com/volumetric-lights/
float get_scattering(float light_dot_view)
{
    float result = 1.0 - scattering * scattering;
    result /= (4.0 * pi * pow(1.0 + scattering * scattering - (2.0 * scattering) * light_dot_view, 1.5));
    return result;
}

void main()
{
    vec2 uv = out_texture_coord;
    vec2 screen_size = textureSize(depth_map, 0);
    float depth = texture(depth_map, uv).r;

    // Raycast from near plane to first object on ray
    vec3 ray_end = world_pos_from_depth(depth, uv);
    vec3 ray_start = world_pos_from_depth(0, uv);

    // Apply bayer matrix
    int index = int(mod(gl_FragCoord.y, 4.0)) * 4 + int(mod(gl_FragCoord.x, 4.0));
    ray_start += dither_pattern[index];
    vec3 ray_direction = normalize(ray_end - ray_start);

    // Work out how far to march each step
    float ray_distance = distance(ray_end, ray_start);
    float step_size = ray_distance / SAMPLES;
    vec3 ray_step = ray_direction * step_size;

    // Work out scattering amount
    vec3 light_direction = -light_position;
    float light_dot_view = dot(ray_direction, light_direction);

    // Begin marching
    vec3 ray_position = ray_start;
    float accumulated = 0.0;

    for (int i = 0; i < SAMPLES; ++i)
    {
        vec4 light_space_position = lightspace * vec4(ray_position, 1.0);
        accumulated += get_scattering(light_dot_view) * get_shadow(light_space_position);

        // March forward; apply bayer matrix
        ray_position += ray_step;
    }

    // Final output
    float volumetrics = accumulated / SAMPLES * intensity;
    frag_colour = volumetrics * vec4(light_colour, 1.0);
}
