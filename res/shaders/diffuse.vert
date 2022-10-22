#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 clip_plane;

out vec2 out_texture_coord;
out vec3 out_normal;
out vec3 out_position;
out mat3 out_tbn;

void main()
{
    // Work out position
    vec4 world_space = model * vec4(pos, 1.0);
    gl_Position = projection * view * world_space;

    // Clipping for planar reflections
    gl_ClipDistance[0] = dot(world_space, clip_plane);

    // Normal mapping
    vec3 bitangent = cross(normal, tangent);
    vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 B = normalize(vec3(model * vec4(bitangent, 0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,    0.0)));

    out_tbn = mat3(T, B, N);
    out_texture_coord = texture_coord;
    out_normal = normal;
    out_position = world_space.xyz;
}
