#version 450
layout(location = 0) in vec3 positionWS;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 colorFS;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec4 color;

layout(set = 0, binding = 1) uniform _FGlobal
{
    vec4 lightPosition;
} FGlobal;


layout(set = 1, binding = 2) uniform sampler2D samplerTex;

void main() 
{
    color =  texture(samplerTex, uv) * vec4(colorFS, 1.0f);
    vec3 L = normalize(FGlobal.lightPosition.xyz - positionWS); 
    vec3 N = normalize(normal);
    float diffuse = max(dot(N, L), 0.0) * FGlobal.lightPosition.w;
    color *= diffuse; 
}