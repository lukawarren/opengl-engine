#version 330 core

in vec2 out_texture_coord;
uniform sampler2D image;
uniform float threshold;
layout (location = 0) out vec4 frag_colour;

void main()
{

    vec4 colour = texture(image, out_texture_coord);
    float brightness = dot(colour.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (brightness > threshold) frag_colour = colour;
    else frag_colour = vec4(0,0,0,1);
}
