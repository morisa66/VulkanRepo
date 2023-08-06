#version 450
layout(location = 0) in vec3 positionWS;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;

layout(location = 0) out vec4 color;

layout(set = 1, binding = 2) uniform sampler2D samplerTex;

void main() 
{
    color =  texture(samplerTex, uv);
}