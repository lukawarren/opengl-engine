#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 out_clip_space;

void main()
{
    out_clip_space = projection * view * model * vec4(pos, 1.0);;
    gl_Position = out_clip_space;
}
