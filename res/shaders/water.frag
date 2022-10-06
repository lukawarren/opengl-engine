#version 330 core

layout (location = 0) out vec4 frag_colour;
in vec2 out_texture_coord;

void main()
{
    frag_colour = vec4(out_texture_coord.x, out_texture_coord.y, 1.0f, 1.0f);
}
