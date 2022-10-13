#version 330 core

in vec2 out_texture_coord;
in vec4 out_clip_space;
in vec3 out_to_camera;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D depth_map;
uniform sampler2D distortion_map;

uniform float z_near;
uniform float z_far;
uniform float time;

layout (location = 0) out vec4 frag_colour;

const float loss = 0.8;
const float waviness = 0.002;
const float fog = 0.2;

float linearise_depth(float d)
{
    return z_near * z_far / (z_far + d * (z_near - z_far));
}

void main()
{
    // Apply du/dv (distortion) texture multiple times
    vec2 distortion1 = texture(distortion_map, vec2(out_texture_coord.x + time, out_texture_coord.y)).xy * 2.0 - 1.0;
    vec2 distortion2 = texture(distortion_map, vec2(out_texture_coord.x, out_texture_coord.y - time)).xy * 2.0 - 1.0;
    vec2 distortion = (distortion1 + distortion2) * waviness;

    // Convert from clip space to "texture-coordinate space"
    vec2 ndc_space = (out_clip_space.xy / out_clip_space.w) / 2.0 + 0.5;

    // Sample depth
    float depth = texture(depth_map, ndc_space).r;
    float distance_to_floor = linearise_depth(depth);
    float distance_to_water = linearise_depth(gl_FragCoord.z);
    float water_depth = distance_to_floor - distance_to_water;

    // Apply distortion for reflection and refraction
    ndc_space += distortion;
    vec4 reflection = texture(reflection_texture, vec2(ndc_space.x, 1 - ndc_space.y));
    vec4 refraction = texture(refraction_texture, ndc_space);

    // Water fog
    float fog = water_depth * fog;
    refraction = mix(refraction, vec4(0,0,0,0), clamp(fog, 0.0, 0.8));

    // Fresnel - bias towards refraction
    float fresnel = dot(normalize(out_to_camera), vec3(0, 1, 0)) + 0.3;
    frag_colour = mix(reflection, refraction, clamp(fresnel, 0, 1));
    frag_colour *= loss;

    // Make water transparent near edges to mask ugly seam
    frag_colour.a = clamp(water_depth*10, 0, 1);
}
