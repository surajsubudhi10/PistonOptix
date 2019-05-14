#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"

rtDeclareVariable(PerRayData, thePrd, rtPayload, );


RT_PROGRAM void miss_environment_constant()
{
	thePrd.radiance = make_float3(1.0f); // Constant white emission. No next event estimation (direct lighting).
	thePrd.flags |= FLAG_TERMINATE;    // End of path.
}
