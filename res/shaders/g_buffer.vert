#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec3 tangent;

uniform mat4 model;
uniform mat4 view_projection;
uniform vec4 clip_plane;

out vec4 out_position;
out vec3 out_normal;
out vec2 out_texture_coord;
out mat3 out_tbn;

void main()
{
    // Work out position
    vec4 world_space = model * vec4(pos, 1.0);
    gl_Position = view_projection * world_space;

    // Clipping for planar reflections
    gl_ClipDistance[0] = dot(world_space, clip_plane);

    // Normal mapping - use the "Gram-Schmidt process" to "re-orthoganalise"
    vec3 T = normalize(vec3(model * vec4(tangent,   0.0)));
    vec3 N = normalize(vec3(model * vec4(normal,    0.0)));
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

    out_position = world_space;
    out_normal = transpose(inverse(mat3(model))) * normal; // Apply model matrix to normal!
    out_texture_coord = texture_coord;
    out_tbn = mat3(T, B, N);
}
