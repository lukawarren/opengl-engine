#version 330 core

in vec2 out_texture_coord;

uniform sampler2D position;
uniform sampler2D normal;
uniform sampler2D depth;
uniform sampler2D noise;
uniform vec3 kernel[16];
uniform vec2 noise_scale;
uniform mat4 view;
uniform mat4 projection;
uniform float radius = 0.5;
uniform float bias = 0.025;
uniform float sharpness = 1.0;

layout (location = 0) out float frag_colour;

void main()
{
    // Sample and convert to view-space
    // NOTE: the "0.0" in the vec4 of the normal makes the effect work better,
    //       and removes artifacts, but is technically incorrect. It just seems
    //       to work better that way...
    vec3 pos = (view * vec4(texture(position, out_texture_coord).xyz, 1.0)).xyz;
    vec3 normal = (view * vec4(texture(normal, out_texture_coord).xyz, 0.0)).xyz;
    vec3 random = texture(noise, out_texture_coord * noise_scale).xyz;

    // Compose TBN matrix
    vec3 tangent = normalize(random - normal * dot(random, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for (int i = 0; i < kernel.length(); ++i)
    {
        // Convert sample position to view-space
        vec3 sample_position = TBN * kernel[i];
        sample_position = pos + sample_position * radius;

        // Transform to screen-space
        vec4 offset = vec4(sample_position, 1.0);
        offset = projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        float sample_depth = (view * vec4(texture(position, offset.xy).xyz, 1.0)).z;
        float range_check = smoothstep(0.0, 1.0, radius / abs(pos.z - sample_depth));
        occlusion += (sample_depth >= sample_position.z + bias ? 1.0 : 0.0) * range_check;
    }

    occlusion = 1.0 - (occlusion / kernel.length());
    frag_colour = pow(occlusion, sharpness);
}
