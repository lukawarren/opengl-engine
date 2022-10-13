#version 330 core

in vec2 out_texture_coord;
in vec3 out_normal;
in vec3 out_position;

uniform sampler2D image;

layout (location = 0) out vec4 frag_colour;

const vec3 light_pos = vec3(5, 10, 3);

void main()
{
    // Diffuse lighting
    vec3 normal = normalize(out_normal);
    vec3 light_direction = normalize(light_pos - out_position);
    float diffuse = max(dot(normal, light_direction), 0.0) * 1.3;
    diffuse = max(diffuse, 0.3);

    // Ignore transparency
    vec4 colour = texture(image, out_texture_coord);
    if (colour.a < 0.5) discard;

    frag_colour = colour * vec4(diffuse, diffuse, diffuse, 1.0);
}
