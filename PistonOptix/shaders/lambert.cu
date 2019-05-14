
#include "per_ray_data.h"
#include "material_parameter.h"
#include "shader_common.h"
#include "PistonOptix/inc/CudaUtils/State.h"

rtDeclareVariable(Ray, theRay, rtCurrentRay, );

RT_CALLABLE_PROGRAM void PDF(MaterialParameter &mat, State &state, PerRayData &prd)
{
	prd.pdf = 0.5f * M_1_PI; // (1 / 2PI)
}

RT_CALLABLE_PROGRAM void Sample(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;				// In World Coordinate
	float3 wo = -theRay.direction;					// In World Coordinate (viewer direction)

	float3 dir = UniformHemisphereSampling(rng2(prd.seed));

	OrthoNormBasis shadingONB(N);
	shadingONB.ToBasisCoordinate(wo);				// In Shading Coordinate

	// if the viewer dir is opossite to surface normal (backface)
	if (wo.z < 0.0f)
		dir.z *= -1.0f;

	shadingONB.ToWorldCoordinate(dir);				// In World coordinate
	
	prd.hit_pos = state.hit_position;
	prd.wi = dir;
}


RT_CALLABLE_PROGRAM float3 Eval(MaterialParameter &mat, State &state, PerRayData &prd)
{
	return mat.albedo * M_1_PIf;
}
