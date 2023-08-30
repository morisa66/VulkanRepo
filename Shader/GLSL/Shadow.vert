#version 450

#include "Core/GlobalVInput.glsl"

layout(set = 1, binding = 0) uniform _VLocal
{
    mat4 model;
    mat4 shadowVP;
}VLocal;

void main()
{
    vec4 positionWordSpace = VLocal.model * vec4(inPosition, 1.0);
    gl_Position = VLocal.shadowVP * positionWordSpace;
    gl_Position.y = -gl_Position.y;
}