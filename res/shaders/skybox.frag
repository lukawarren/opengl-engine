#version 330 core

in vec3 tex_coords;

uniform samplerCube skybox;
uniform sampler2D depth;
uniform vec2 screen_size;
uniform vec3 tint;

out vec4 frag_colour;

void main()
{
    // UVs for below
    vec2 screen_space = gl_FragCoord.xy / screen_size;

    // Only render if not behind other objects
    // (done this way as we're a deferred renderer)
    if (texture(depth, screen_space.xy).r != 1)
        discard;

    frag_colour = texture(skybox, tex_coords) * vec4(tint, 1.0);
}
