#version 330 core

layout (location = 0) out vec4 frag_colour;
uniform sampler2D image;
in vec2 out_texture_coord;

void main()
{
    vec2 uv = out_texture_coord;
    frag_colour = texture(image, uv);
}
