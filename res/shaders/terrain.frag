#version 330 core

in vec2 out_texture_coord;
in vec3 out_normal;
in vec3 out_position;

uniform sampler2D image;

uniform vec3 ambient_light;
uniform vec3 light_direction;
uniform vec3 light_colour;

layout (location = 0) out vec4 frag_colour;

const vec3 light_pos = vec3(5, 10, 3);

void main()
{
    // Diffuse lighting
    vec3 normal = normalize(out_normal);
    vec3 diffuse = max(dot(normal, -light_direction), 0.0) * light_colour;
    diffuse += ambient_light;

    // Ignore transparency
    vec4 colour = texture(image, out_texture_coord);
    if (colour.a < 0.5) discard;

    frag_colour = colour * vec4(diffuse, 1.0);
}
