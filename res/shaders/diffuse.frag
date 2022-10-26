#version 330 core

in vec2 out_texture_coord;
in vec3 out_normal;
in vec3 out_position;
in mat3 out_tbn;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform bool has_normal_map;

uniform vec3 ambient_light;
uniform vec3 light_direction;
uniform vec3 light_colour;

layout (location = 0) out vec4 frag_colour;

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

    // Diffuse lighting
    vec3 diffuse = max(dot(normal, -light_direction), 0.0) * light_colour;
    diffuse += ambient_light;

    // Ignore transparency
    vec4 colour = texture(diffuse_map, out_texture_coord);
    if (colour.a < 0.5) discard;

    frag_colour = colour * vec4(diffuse, 1.0);
}
