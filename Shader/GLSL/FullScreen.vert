#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

layout(location = 0) out vec2 uv;

void main() 
{
    gl_Position = vec4(inPosition, 1.0);
    uv = inUV;
    gl_Position.y = -gl_Position.y;
}