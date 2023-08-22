#version 450
layout(location = 0) in vec3 positionWS;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 colorFS;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 shadowPositionCS;

layout(location = 0) out vec4 finalColor;

layout(set = 0, binding = 1) uniform _FGlobal
{
    vec4 lightPosition;
} FGlobal;


layout(set = 1, binding = 2) uniform sampler2D Samplers[];

const uint MainTex = 0;
const uint ShadowMap = 1;

const float ambient = 0.1f;

void main() 
{
    vec4 mainColor = texture(Samplers[MainTex], uv);
    vec4 color =  mainColor * vec4(colorFS, 1.0f);
    vec3 L = normalize(FGlobal.lightPosition.xyz - positionWS); 
    vec3 N = normalize(normal);
    float diffuse = max(ambient, dot(N, L)) * FGlobal.lightPosition.w;

	vec4 divShadowPositionCS = shadowPositionCS / shadowPositionCS.w;
	vec2 shadowUV = divShadowPositionCS.xy * 0.5f + 0.5f;
	float shadowZ = divShadowPositionCS.z;
	float shadow = texture(Samplers[ShadowMap], shadowUV).r > shadowZ ? 1.0f : ambient;
	finalColor = color * diffuse * shadow;
}