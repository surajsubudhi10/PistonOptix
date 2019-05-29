#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"
#include "shader_common.h"
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

RT_CALLABLE_PROGRAM void sphere_sample(LightParameter &light, PerRayData &prd, LightSample &sample)
{
	const float r1 = rng(prd.seed);
	const float r2 = rng(prd.seed);
	sample.surfacePos = light.position + UniformSampleSphere(r1, r2) * light.radius;
	sample.direction = normalize(sample.surfacePos - light.position);
	sample.emission = light.emission * sysNumberOfLights;
	sample.pdf = -1.0f;
}

RT_CALLABLE_PROGRAM void quad_sample(LightParameter &light, PerRayData &prd, LightSample &sample)
{
	const float r1 = rng(prd.seed);
	const float r2 = rng(prd.seed);
	sample.surfacePos = light.position + light.u * r1 + light.v * r2;
	sample.direction = light.direction;
	sample.emission = light.emission * sysNumberOfLights;
	sample.pdf = -1.0f;
}

RT_CALLABLE_PROGRAM void directional_sample(LightParameter &light, PerRayData &prd, LightSample &sample)
{
	sample.direction = light.direction;
	sample.emission = light.emission * sysNumberOfLights;
	sample.pdf = 1.0f;
}
