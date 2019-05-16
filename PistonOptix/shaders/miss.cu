#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"

rtDeclareVariable(PerRayData, thePrd, rtPayload, );
rtDeclareVariable(optix::Ray, ray, rtCurrentRay, );

rtTextureSampler<float4, 2> envmap;


RT_PROGRAM void miss_environment_constant()
{
	float theta = atan2f(ray.direction.x, ray.direction.z);
	float phi = M_PIf * 0.5f - acosf(ray.direction.y);
	float u = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v = 0.5f * (1.0f + sin(phi));
	float3 result = make_float3(tex2D(envmap, u, v));

	thePrd.radiance = result;// make_float3(1.0f); // Constant white emission. No next event estimation (direct lighting).
	thePrd.flags |= FLAG_TERMINATE;    // End of path.
}
