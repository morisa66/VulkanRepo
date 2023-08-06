#version 450
layout(location = 0) in vec2 uv;

layout(location = 0) out vec4 finalColor;

layout(set = 1, binding = 2) uniform sampler2D Samplers[];

const uint ColorTex = 0;
const uint DepthTex = 1;
const uint BaseMapTex = 2;

void main() 
{
    vec2 inverUV = vec2(uv.x, 1.0f - uv.y);
    vec4 color = texture(Samplers[ColorTex],  inverUV);
    float depth =  texture(Samplers[DepthTex],  inverUV).r;
    vec4 baseColor =  texture(Samplers[BaseMapTex],  uv);
    finalColor = vec4(color);
}