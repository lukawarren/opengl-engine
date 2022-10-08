#version 330 core

in vec2 out_texture_coord;
in vec4 out_clip_space;
in vec3 out_to_camera;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D distortion_texture;
uniform float time;

layout (location = 0) out vec4 frag_colour;

const float energy_loss = 0.8;

void main()
{
    // Apply du/dv (distortion) texture multiple times
    vec2 distortion1 = texture(distortion_texture, vec2(out_texture_coord.x + time, out_texture_coord.y)).xy * 2.0 - 1.0;
    vec2 distortion2 = texture(distortion_texture, vec2(out_texture_coord.x, out_texture_coord.y - time)).xy * 2.0 - 1.0;
    vec2 distortion = (distortion1 + distortion2) * 0.005;

    // Convert from clip space to "texture-coordinate space"
    vec2 ndc_space = (out_clip_space.xy / out_clip_space.w) / 2.0 + 0.5;
    ndc_space += distortion;
    vec4 reflection = texture(reflection_texture, vec2(ndc_space.x, 1 - ndc_space.y));
    vec4 refraction = texture(refraction_texture, ndc_space);

    // Fresnel
    float fresnel = dot(normalize(out_to_camera), vec3(0, 1, 0));

    // Water doesn't reflect 100% of light :)
    vec4 loss = vec4(energy_loss, energy_loss, energy_loss, 1.0);
    frag_colour = mix(reflection, refraction, fresnel) * loss;
}
