#version 330 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 texture_coord;
layout (location = 2) in vec3 normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec4 clip_plane;

out vec2 out_texture_coord;
out vec3 out_normal;
out vec3 out_position;

/*
    Temporary simplex noise from https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83
*/

vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
  const vec4 C = vec4(0.211324865405187, 0.366025403784439,
           -0.577350269189626, 0.024390243902439);
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);
  vec2 i1;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;
  i = mod(i, 289.0);
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
  + i.x + vec3(0.0, i1.x, 1.0 ));
  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy),
    dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;
  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}

float height(vec2 position)
{
    /*float p1 = snoise(position * 0.1);
    float p2 = snoise(position * 0.4);
    float p3 = snoise(position * 0.7);
    float p4 = snoise(position * 1.0);
    return (p1+p2+p3+p4) * 2;
    */
    return snoise(position * 0.05) * 2
        + snoise(position * 0.5 + vec2(50,100)) * 0.1
        + snoise(position * 5.0) * 0.1;
}

void main()
{
    vec4 world_space = model * vec4(pos.x, height(pos.xz), pos.z, 1.0);

    gl_Position = projection * view * world_space;
    gl_ClipDistance[0] = dot(world_space, clip_plane);

    out_texture_coord = texture_coord;
    out_normal = normal;
    out_position = world_space.xyz;
}
