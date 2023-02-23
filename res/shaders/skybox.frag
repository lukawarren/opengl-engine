#version 330 core

in vec3 tex_coords;
uniform samplerCube skybox;
uniform vec3 tint;
out vec4 frag_colour;

void main()
{
    frag_colour = texture(skybox, tex_coords) * vec4(tint, 1.0);
}
