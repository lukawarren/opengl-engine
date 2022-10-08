#version 330 core

in vec2 out_texture_coord;
in vec3 out_normal;
in vec3 out_position;

uniform sampler2D image;

layout (location = 0) out vec4 frag_colour;

void main()
{
    // Diffuse lighting
    vec3 normal = normalize(out_normal);
    vec3 light_direction = normalize(vec3(10, 5, 3) - out_position);
    float diffuse = max(dot(normal, light_direction), 0.0);

    frag_colour = texture(image, out_texture_coord) * vec4(diffuse, diffuse, diffuse, 1.0);
}
