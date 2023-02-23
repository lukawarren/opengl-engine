#version 330 core

layout (location = 0) in vec3 pos;
uniform mat4 matrix;
out vec3 tex_coords;

void main()
{
    tex_coords = pos;
    gl_Position = matrix * vec4(pos, 1.0);
}
