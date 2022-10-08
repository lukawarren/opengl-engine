#version 330 core

in vec4 out_clip_space;
uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
layout (location = 0) out vec4 frag_colour;

void main()
{
    // Convert from clip space to "texture-coordinate space"
    vec2 ndc_space = (out_clip_space.xy / out_clip_space.w) / 2.0 + 0.5;

    vec4 reflection = texture(reflection_texture, ndc_space * vec2(1.0, -1.0));
    vec4 refraction = texture(refraction_texture, ndc_space);
    frag_colour = mix(reflection, refraction, 0.5);
}
