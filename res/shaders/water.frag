#version 330 core

in vec2 out_texture_coord;
in vec4 out_clip_space;
in vec3 out_to_camera;
in vec3 out_from_light;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;
uniform sampler2D depth_map;
uniform sampler2D distortion_map;
uniform sampler2D normal_map;

uniform vec3 light_direction;
uniform vec3 light_colour;

uniform float z_near;
uniform float z_far;
uniform float time;

layout (location = 0) out vec4 frag_colour;

const float waviness = 0.002;
const float loss = 0.8;
const float fog = 0.2;

const float specular_damper = 20.0;
const float specular_factor = 0.6;

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

    // Normal mapping
    vec4 normal_sample = texture(normal_map, distortion);
    vec3 normal = vec3(normal_sample.r * 2.0 - 1.0, normal_sample.b, normal_sample.g * 2.0 -  1.0);
    normal = normalize(normal);

    // Water fog
    float fog = water_depth * fog;
    refraction = mix(refraction, vec4(0,0,0,0), clamp(fog, 0.0, 0.8));

    // Fresnel - bias towards refraction
    vec3 view_vector = normalize(out_to_camera);
    float fresnel = dot(view_vector, normal) + 0.3;
    frag_colour = mix(reflection, refraction, clamp(fresnel, 0, 1));
    frag_colour *= loss;

    // Specular lighting
    vec3 reflected_light = reflect(normalize(out_from_light), normal);
    float specular = max(dot(reflected_light, view_vector), 0.0);
    specular = pow(specular, specular_damper) * specular_factor;
    frag_colour += vec4(specular * light_colour, 0);

    // Make water transparent near edges to mask ugly seam
    frag_colour.a = clamp(water_depth*10, 0, 1);
}
