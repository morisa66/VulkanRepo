#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;


layout(set = 0, binding = 0) uniform _VGlobal
{
    mat4 view;
    mat4 proj;
} VGlobal;

layout(set = 1, binding = 0) uniform _VLocal
{
    mat4 model;
}VLocal;

layout(location = 0) out vec3 positionWS;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec2 uv;

void main() 
{
    vec4 positionWordSpace = VLocal.model * vec4(inPosition, 1.0);
    gl_Position = VGlobal.proj * VGlobal.view * positionWordSpace;
    positionWS = positionWordSpace.xyz;
    normal = inNormal;
    uv = inUV;

    gl_Position.y = -gl_Position.y;
}