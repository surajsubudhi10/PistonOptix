#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"
#include "shader_common.h"
#include "..\inc\CudaUtils\State.h"
#include "..\inc\LightParameters.h"

#include "rt_assert.h"

rtDeclareVariable(int, sysNumberOfLights, , );

RT_FUNCTION float3 UniformSampleSphere(float u1, float u2)
{
	float z = 1.f - 2.f * u1;
	float r = sqrtf(max(0.f, 1.f - z * z));
	float phi = 2.f * M_PIf * u2;
	float x = r * cosf(phi);
	float y = r * sinf(phi);

	return make_float3(x, y, z);
}

RT_CALLABLE_PROGRAM void sphere_sample(POptix::Light &light, PerRayData &prd, POptix::LightSample &sample, State& state)
{
	const float r1 = rng(prd.seed);
	const float r2 = rng(prd.seed);
	sample.surfacePos = light.position;// +UniformSampleSphere(r1, r2) * light.radius;
	sample.direction = normalize(sample.surfacePos - state.hit_position);
	rtPrintf("sample.direction : %f, %f, %f\n", sample.direction.x, sample.direction.y, sample.direction.z);
	sample.emission = light.emission * sysNumberOfLights;
	sample.distance = length(light.position - state.hit_position);

	//float NdotL = dot(lightSample.direction, -lightDir);
	float lightDistSq = sample.distance * sample.distance;
	sample.pdf = lightDistSq / (light.area);
}

RT_CALLABLE_PROGRAM void directional_sample(POptix::Light &light, PerRayData &prd, POptix::LightSample &sample, State& state)
{
	sample.direction = -light.normal;
	sample.distance = RT_DEFAULT_MAX;
	sample.emission = light.emission * sysNumberOfLights;
	sample.pdf = 1.0f;
}


RT_CALLABLE_PROGRAM void quad_sample(POptix::Light &light, PerRayData &prd, POptix::LightSample &sample, State& state) 
{
	const float r1 = rng(prd.seed);
	const float r2 = rng(prd.seed);
	
	// position on the area light
	sample.surfacePos = light.position + light.u * r1 + light.v * r2;
	sample.pdf = 1.0f / light.area;

	// light ray direction
	float3 wi = normalize(sample.surfacePos - state.hit_position);
	float distance = length(sample.surfacePos - state.hit_position);

	float cosTheta = fabsf(dot(light.normal, -wi));
	if (cosTheta < DENOMINATOR_EPSILON) 
	{
		sample.pdf = 0.0f;
		return;
	}

	sample.pdf *= (distance * distance) / cosTheta;
	if (sample.pdf > 10E19) 
		sample.pdf = 0.0f;

	sample.distance = distance;
	sample.direction = wi;
	sample.emission = light.emission;
}