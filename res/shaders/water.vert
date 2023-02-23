#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 camera_position;
uniform vec3 light_position;

out vec4 out_clip_space;
out vec3 out_to_camera;
out vec3 out_from_light;
out vec2 out_texture_coord;

void main()
{
    vec4 world_space = model * vec4(pos, 1.0);
    out_to_camera = camera_position - world_space.xyz;
    out_from_light = world_space.xyz - light_position;
    out_clip_space = projection * view * world_space;
    gl_Position = out_clip_space;
    out_texture_coord = texture_coord * 60; // Tiling du/dv map
}
