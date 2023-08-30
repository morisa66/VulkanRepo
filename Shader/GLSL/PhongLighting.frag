#version 450
layout(location = 0) in vec3 positionWS;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec3 colorFS;
layout(location = 3) in vec2 uv;
layout(location = 4) in vec4 shadowPositionCS;

layout(location = 0) out vec4 finalColor;

#include "Core/GlobalFInput.glsl"
#include "Shadow/ShadowCommon.glsl"

layout(set = 1, binding = 1) uniform _FLocal
{
    vec4 shadowParams;
} FLocal;

layout(set = 1, binding = 2) uniform sampler2D Samplers[];

const uint MainTex = 0;
const uint ShadowMap = 1;

void main() 
{
    vec4 mainColor = texture(Samplers[MainTex], uv);
    vec4 color =  mainColor * vec4(colorFS, 1.0f);
    vec3 L = FGlobal.lightPosition.xyz - positionWS;
    float attenuation = 1.0f / (1.0f + dot(L, L));
    L = normalize(L); 
    vec3 N = normalize(normal);
	float NoL = dot(N, L);
    float diffuse = max(ambient, NoL) * FGlobal.lightPosition.w * attenuation;

	vec4 divShadowPositionCS = shadowPositionCS / shadowPositionCS.w;
	vec2 shadowUV = divShadowPositionCS.xy * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y;
	float shadowZ = divShadowPositionCS.z;
	vec2 shadowMapSize = FLocal.shadowParams.xy;
	float averageDepthFilterSize = FLocal.shadowParams.z;
	float lightScale = FLocal.shadowParams.w;

    PoissonDiskSampleInit(shadowUV);

	float blockAverageDepth = BlockAverageDepth(Samplers[ShadowMap], shadowMapSize, shadowUV, shadowZ, averageDepthFilterSize);

	float shadow = 1.0f;
	if(blockAverageDepth < shadowZ)
	{
		float penumbraScale = (shadowZ - blockAverageDepth) * lightScale / blockAverageDepth;
		shadow = PCF(Samplers[ShadowMap], shadowMapSize, shadowUV, shadowZ, NoL, penumbraScale);
	}
	finalColor = mainColor * diffuse * shadow;
}