#version 330 core

in vec2 out_texture_coord;
uniform sampler2D image_one;
uniform sampler2D image_two;
uniform sampler2D image_three;
layout (location = 0) out vec4 frag_colour;

const float gamma = 2.2;

// ACES film tonemapping
vec3 map_hdr(vec3 x)
{
	const float a = 2.51f;
	const float b = 0.03f;
	const float c = 2.43f;
	const float d = 0.59f;
	const float e = 0.14f;
	return clamp(((x*(a*x+b)) / (x*(c*x+d)+e)), 0.0, 1.0);
}

void main()
{
    vec2 uv = out_texture_coord;
    vec3 hdr_one = texture(image_one, uv).xyz;
    vec3 hdr_two = texture(image_two, uv).xyz;
    vec4 hdr_three = texture(image_three, uv);

    // hdr_three is from clouds, so blend accordingly
    vec3 hdr = hdr_one + hdr_two;
    hdr = mix(hdr_three.xyz, hdr, hdr_three.a);

    // Exposure tone mapping + gamma correction
    vec3 mapped = map_hdr(hdr);
    mapped = pow(mapped, vec3(1.0 / gamma));
    frag_colour = vec4(mapped, 1.0);
}
