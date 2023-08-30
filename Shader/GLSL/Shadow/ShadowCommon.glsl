const float ambient = 0.2f;
const float PI = 3.14159265359;
const int SAMPLE_COUNT = 20;
const int RING_COUNT = 10;

vec2 PoissonDiskSamples[SAMPLE_COUNT];

float rand(vec2 seed) 
{
	return fract(sin(mod(dot(uv.xy, vec2(12.9898f, 78.233f)), PI)) * 43758.5453f);
}

void PoissonDiskSampleInit(vec2 uv) 
{
    const float RADIUS_STEP = 1.0f / float(SAMPLE_COUNT);
    const float ANGLE_STEP = 2.0f * PI * float(RING_COUNT) * RADIUS_STEP;
    float radius = RADIUS_STEP;
    float angle = 2.0f * PI * rand(uv);

    for(int i = 0; i < SAMPLE_COUNT; i++)
    {
        PoissonDiskSamples[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += RADIUS_STEP;
        angle += ANGLE_STEP;
    }
}

float DepthBias(float NoL)
{
    return 0.0001f * (1.0f - NoL);
}

float SampleShadow(sampler2D shadowMap, vec2 uv, float shadowZ, float NoL)
{
    return texture(shadowMap, uv).r + DepthBias(NoL) < shadowZ ?  ambient : 1.0f;
}

float PCF(sampler2D shadowMap, vec2 shadowMapSize, vec2 uv, float shadowZ, float NoL, float radius)
{
    const vec2 FILTER_SIZE = radius / shadowMapSize;
	float shadowSum = 0.0;
    for(int i = 0; i < SAMPLE_COUNT; i++)
    {
        shadowSum += SampleShadow(shadowMap, uv + PoissonDiskSamples[i] * FILTER_SIZE, shadowZ, NoL);
    }
	return shadowSum / float(SAMPLE_COUNT);
}

float BlockAverageDepth(sampler2D shadowMap, vec2 shadowMapSize, vec2 uv, float shadowZ, float radius)
{
    const vec2 FILTER_SIZE = radius / shadowMapSize;
    int blockCount = 0;
    float depthSum = 0.0f;
    for(int i = 0; i < SAMPLE_COUNT; i++)
    {
        float depthCurrent = texture(shadowMap, uv + PoissonDiskSamples[i] * FILTER_SIZE).r;
        if(depthCurrent < shadowZ)
        {
            blockCount++;
            depthSum += depthCurrent;
        }
    }
    return blockCount > 0 ? depthSum / float(blockCount) : 1.0f;
}