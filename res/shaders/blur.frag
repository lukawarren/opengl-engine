#version 330 core

in vec2 out_texture_coord;

uniform sampler2D image;
uniform bool horizontal;

const float weights[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

layout (location = 0) out vec4 frag_colour;

void main()
{
    vec2 uv = out_texture_coord;
    vec2 texel_size = 1.0 / textureSize(image, 0);
    vec4 result = texture(image, uv) * weights[0];

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, uv + vec2(texel_size.x * i, 0.0)) * weights[i];
            result += texture(image, uv - vec2(texel_size.x * i, 0.0)) * weights[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(image, uv + vec2(0.0, texel_size.y * i)) * weights[i];
            result += texture(image, uv - vec2(0.0, texel_size.y * i)) * weights[i];
        }
    }

    frag_colour = result;
}
