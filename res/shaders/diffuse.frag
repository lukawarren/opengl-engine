#version 330 core

in vec2 out_texture_coord;
in vec3 out_normal;
in vec3 out_position;
in mat3 out_tbn;
in vec4 out_lightspace_position;

uniform sampler2D diffuse_map;
uniform sampler2D shadow_map;
uniform sampler2D normal_map;
uniform bool has_normal_map;

uniform vec3 ambient_light;
uniform vec3 light_position;
uniform vec3 light_colour;

layout (location = 0) out vec4 frag_colour;

float get_shadow(vec4 lightspace_position)
{
    // Perspective divide
    vec3 proj_coords = lightspace_position.xyz / lightspace_position.w;

    // Transform to [0, 1] range
    proj_coords = proj_coords * 0.5 + 0.5;
    float current_depth = proj_coords.z;

    // If outside of shadow map, ditch
    if(proj_coords.z > 1.0)
        return 1;

    // PCF
    float bias = 0.00005;

    float shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcf_depth = texture(shadow_map, proj_coords.xy + vec2(x, y) * texel_size).r;

            // Finally sample depth (i.e. the important bit)
            // NOTE: 0.1 is used instead of 0.0 to provide ambient lighting
            shadow += current_depth - bias > pcf_depth ? 0.1 : 1.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    // Normal mapping
    vec3 normal;
    if (has_normal_map)
    {
        normal = texture(normal_map, out_texture_coord).rgb;
        normal = normal * 2.0 - 1.0;
        normal = normalize(out_tbn * normal);
    }
    else
        normal = normalize(out_normal);


    // Diffuse lighting - assume light to be a direction (e.g. the sun and i.e. not a point light)
    vec3 light_direction = normalize(light_position);
    vec3 diffuse = max(dot(normal, light_direction), 0.0) * light_colour;
    diffuse += ambient_light;

    // Ignore transparency
    vec4 colour = texture(diffuse_map, out_texture_coord);
    if (colour.a < 0.5) discard;

    // Shadows
    float shadow = get_shadow(out_lightspace_position);
    frag_colour = colour * vec4(diffuse, 1.0) * shadow;
}
