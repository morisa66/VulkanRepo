#version 450

#include "Core/GlobalVInput.glsl"

layout(set = 1, binding = 0) uniform _VLocal
{
    mat4 model;
    mat4 shadowVP;
}VLocal;

layout(location = 0) out vec3 positionWS;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec3 colorFS;
layout(location = 3) out vec2 uv;
layout(location = 4) out vec4 shadowPositionCS;

void main() 
{
    vec4 positionWordSpace = VLocal.model * vec4(inPosition, 1.0);
    gl_Position = VGlobal.proj * VGlobal.view * positionWordSpace;
    positionWS = positionWordSpace.xyz;
    normal = inNormal;
    colorFS = inColor;
    uv = inUV;
    shadowPositionCS =  VLocal.shadowVP * positionWordSpace;
    gl_Position.y = -gl_Position.y;
}