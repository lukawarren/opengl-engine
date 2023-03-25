#version 330 core

in vec2 out_texture_coord;

uniform sampler2D g_albedo;
uniform sampler2D g_normal;
uniform sampler2D g_position;
uniform sampler2D occlusion;
uniform sampler2D shadow_map;
uniform sampler3D cloud_map;

uniform vec3 ambient_light;
uniform vec3 light_position;
uniform vec3 light_colour;
uniform mat4 lightspace;

uniform float cloud_scale;
uniform float cloud_offset;

layout (location = 0) out vec4 frag_colour;

const vec2 poisson_disk[64] = vec2[]
(
    vec2(-0.613392,  0.617481),
    vec2( 0.170019, -0.040254),
    vec2(-0.299417,  0.791925),
    vec2( 0.645680,  0.493210),
    vec2(-0.651784,  0.717887),
    vec2( 0.421003,  0.027070),
    vec2(-0.817194, -0.271096),
    vec2(-0.705374, -0.668203),
    vec2( 0.977050, -0.108615),
    vec2( 0.063326,  0.142369),
    vec2( 0.203528,  0.214331),
    vec2(-0.667531,  0.326090),
    vec2(-0.098422, -0.295755),
    vec2(-0.885922,  0.215369),
    vec2( 0.566637,  0.605213),
    vec2( 0.039766, -0.396100),
    vec2( 0.751946,  0.453352),
    vec2( 0.078707, -0.715323),
    vec2(-0.075838, -0.529344),
    vec2( 0.724479, -0.580798),
    vec2( 0.222999, -0.215125),
    vec2(-0.467574, -0.405438),
    vec2(-0.248268, -0.814753),
    vec2( 0.354411, -0.887570),
    vec2( 0.175817,  0.382366),
    vec2( 0.487472, -0.063082),
    vec2(-0.084078,  0.898312),
    vec2( 0.488876, -0.783441),
    vec2( 0.470016,  0.217933),
    vec2(-0.696890, -0.549791),
    vec2(-0.149693,  0.605762),
    vec2( 0.034211,  0.979980),
    vec2( 0.503098, -0.308878),
    vec2(-0.016205, -0.872921),
    vec2( 0.385784, -0.393902),
    vec2(-0.146886, -0.859249),
    vec2( 0.643361,  0.164098),
    vec2( 0.634388, -0.049471),
    vec2(-0.688894,  0.007843),
    vec2( 0.464034, -0.188818),
    vec2(-0.440840,  0.137486),
    vec2( 0.364483,  0.511704),
    vec2( 0.034028,  0.325968),
    vec2( 0.099094, -0.308023),
    vec2( 0.693960, -0.366253),
    vec2( 0.678884, -0.204688),
    vec2( 0.001801,  0.780328),
    vec2( 0.145177, -0.898984),
    vec2( 0.062655, -0.611866),
    vec2( 0.315226, -0.604297),
    vec2(-0.780145,  0.486251),
    vec2(-0.371868,  0.882138),
    vec2( 0.200476,  0.494430),
    vec2(-0.494552, -0.711051),
    vec2( 0.612476,  0.705252),
    vec2(-0.578845, -0.768792),
    vec2(-0.772454, -0.090976),
    vec2( 0.504440,  0.372295),
    vec2( 0.155736,  0.065157),
    vec2( 0.391522,  0.849605),
    vec2(-0.620106, -0.328104),
    vec2( 0.789239, -0.419965),
    vec2(-0.545396,  0.538133),
    vec2(-0.178564, -0.596057)
);

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

    // Soft shadows with PCF + stratified Poisson sampling
    const int samples = 64;
    float total_shadow = 0.0;
    vec2 texel_size = 1.0 / textureSize(shadow_map, 0);
    float spread = 2.5;

    for (int i = 0; i < samples; ++i)
    {
        const float bias = 0.0001;
        float depth = texture(shadow_map, proj_coords.xy + poisson_disk[i] * texel_size * spread).r;
        float shadow = step(current_depth - bias, depth);
        total_shadow += shadow;
    }

    return total_shadow / samples;
}

float get_cloud_shadow(vec3 world_position)
{
    vec2 cloud_pos = (world_position.xz + cloud_offset) * 0.01 * cloud_scale;
    float density = texture(cloud_map, vec3(cloud_pos, 0)).r;
    density = pow(density, 4) * 2;
    return max(1.0 - density, 0);
}

void main()
{
    // Sample g-buffer
    vec3 albedo = texture(g_albedo, out_texture_coord).xyz;
    vec3 normal = texture(g_normal, out_texture_coord).xyz;
    vec3 position = texture(g_position, out_texture_coord).xyz;
    vec4 lightspace_position = lightspace * vec4(position, 1.0);
    float occlusion = texture(occlusion, out_texture_coord).r;

    // Ambient lighting
    occlusion = max(occlusion, 0.5);
    vec3 ambience = ambient_light * occlusion;

    // Diffuse lighting - assume light to be a direction (e.g. the sun and i.e. not a point light)
    vec3 light_direction = normalize(light_position);
    vec3 diffuse = max(dot(normal, light_direction), 0.0) * light_colour;
    diffuse += ambience;

    // Shadows
    float shadow = max(get_shadow(lightspace_position), 0.4 * occlusion);
    float cloud_shadow = max(get_cloud_shadow(position.xyz), 0.3 * occlusion);
    frag_colour = vec4(albedo, 1.0) * vec4(diffuse, 1.0) * shadow * cloud_shadow;
}
