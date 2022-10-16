#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 clip_plane;

out vec2 out_texture_coord;
out vec3 out_normal;
out vec3 out_position;

void main()
{
    vec4 world_space = model * vec4(pos, 1.0);

    gl_Position = projection * view * world_space;
    gl_ClipDistance[0] = dot(world_space, clip_plane);

    out_texture_coord = texture_coord;
    out_normal = normal;
    out_position = world_space.xyz;
}
