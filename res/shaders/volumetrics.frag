#version 330 core

in vec2 out_texture_coord;

uniform mat4 inverse_view;
uniform mat4 inverse_projection;
uniform mat4 lightspace;
uniform vec3 light_position;
uniform vec3 light_colour;
uniform sampler2D image;
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
    return current_depth > closest_depth ? 1.0 : 0.0;
}

const int SAMPLES = 50;
const float intensity = 0.7;
const float max_distance = 20.0;

void main()
{
    vec2 uv = out_texture_coord * vec2(1, -1);
    vec2 screen_size = textureSize(depth_map, 0);
    float depth = texture(depth_map, uv).r;

    // Raycast from near plane to first object on ray
    vec3 ray_end = world_pos_from_depth(depth, uv);
    vec3 ray_start = world_pos_from_depth(0, uv);
    vec3 ray_direction = normalize(ray_end - ray_start);

    // Work out how far to march each step
    float ray_distance = distance(ray_end, ray_start);
    float step_size = ray_distance / SAMPLES;
    vec3 ray_step = ray_direction * step_size;

    // Begin marching
    vec3 ray_position = ray_start;
    float accumulated = 0.0;
    for (int i = 0; i < SAMPLES; ++i)
    {
        vec4 light_space_position = lightspace * vec4(ray_position, 1.0);
        accumulated += 1.0 - get_shadow(light_space_position);
        ray_position += ray_step;
    }

    // Final output
    float volumetrics = (ray_distance / max_distance) * intensity * (accumulated / SAMPLES);
    vec4 original = texture(image, uv);
    frag_colour = volumetrics * vec4(light_colour, 1.0) + original;
}
