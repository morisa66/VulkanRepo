#version 450

#include "Core/GlobalVInput.glsl"

layout(location = 0) out vec2 uv;

void main() 
{
    gl_Position = vec4(inPosition, 1.0);
    uv = inUV;
    gl_Position.y = -gl_Position.y;
}