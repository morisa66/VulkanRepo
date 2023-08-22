#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inColor;
layout(location = 3) in vec2 inUV;

layout(set = 1, binding = 0) uniform _VLocal
{
    mat4 model;
    mat4 shadowVP;
}VLocal;

void main()
{
    vec4 positionWordSpace = VLocal.model * vec4(inPosition, 1.0);
    gl_Position = VLocal.shadowVP * positionWordSpace;
}