#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_position;

out vec4 out_clip_space;
out vec3 out_to_camera;
out vec2 out_texture_coord;

const vec3 light_pos = vec3(5, 10, 3);

void main()
{
    vec4 world_space = model * vec4(pos, 1.0);
    out_to_camera = camera_position - world_space.xyz;
    out_clip_space = projection * view * world_space;
    gl_Position = out_clip_space;
    out_texture_coord = texture_coord * 3; // Tiling du/dv map
}
