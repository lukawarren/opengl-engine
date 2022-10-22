#version 330 core

in vec2 out_texture_coord;
in vec3 out_normal;
in vec3 out_position;
in mat3 out_tbn;

uniform sampler2D diffuse_map;
uniform sampler2D normal_map;
uniform bool has_normal_map;

layout (location = 0) out vec4 frag_colour;

const vec3 light_pos = vec3(5, 10, 3);

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
    vec3 light_direction = normalize(light_pos - out_position);
    float diffuse = max(dot(normal, light_direction), 0.0) * 1.3;
    diffuse = max(diffuse, 0.3);

    // Ignore transparency
    vec4 colour = texture(diffuse_map, out_texture_coord);
    if (colour.a < 0.5) discard;

    frag_colour = colour * vec4(diffuse, diffuse, diffuse, 1.0);
}
