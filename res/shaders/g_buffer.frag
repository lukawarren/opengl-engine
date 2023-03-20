#version 330 core

in vec4 out_position;
in vec3 out_normal;
in vec2 out_texture_coord;
in mat3 out_tbn;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform bool has_normal_map;

layout (location = 0) out vec3 g_albedo;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec3 g_position;

void main()
{
    // Normal mapping
    vec3 normal;
    if (has_normal_map)
    {
        normal = texture(normal_map, out_texture_coord).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(out_tbn * normal);
    }
    else
        normal = normalize(out_normal);

    // Ignore transparency
    vec4 colour = texture(diffuse_map, out_texture_coord);
    if (colour.a < 0.5) discard;

    g_albedo = colour.xyz;
    g_normal = normal;
    g_position = out_position.xyz;
}
