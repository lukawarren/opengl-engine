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

const float energy_loss = 0.8; // can be thought of as the water's "dirtiness"
const vec4 colour = vec4(0.0/255.0, 167.0/255.0, 255.0/255.0, 1.0); // water base colour
const float fog_factor = 20;
const float max_fog_damper = 0.8; // higher = water can get darker
const float waviness = 0.003;

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
    float distance_to_floor = 2.0 * z_near * z_far / (z_far + z_near - (2.0 * depth - 1.0) * (z_far - z_near));
    float distance_to_water = 2.0 * z_near * z_far / (z_far + z_near - (2.0 * gl_FragCoord.z - 1.0) * (z_far - z_near));
    float water_depth = distance_to_floor - distance_to_water;

    // Apply distortion for reflection and refraction
    ndc_space += distortion;
    vec4 reflection = texture(reflection_texture, vec2(ndc_space.x, 1 - ndc_space.y));
    vec4 refraction = texture(refraction_texture, ndc_space);

    // Use water depth to dull refractions (fog)
    float loss = clamp(water_depth * fog_factor, 0, 1.0);
    loss = clamp(loss, 0, max_fog_damper);
    refraction *= (1-loss);

    // Fresnel - bias towards refraction
    float fresnel = dot(normalize(out_to_camera), vec3(0, 1, 0));
    fresnel = clamp(fresnel * 1.5, 0, 1);
    frag_colour = mix(reflection, refraction, fresnel);

    // Water doesn't reflect 100% of light :)
    frag_colour = frag_colour * energy_loss * colour;

    // Make water transparent near edges to mask ugly seam
    frag_colour.a = clamp(water_depth*2, 0, 1);
}
