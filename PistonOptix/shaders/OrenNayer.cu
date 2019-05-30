
#include "per_ray_data.h"
#include "material_parameter.h"
#include "shader_common.h"
#include "PistonOptix/inc/CudaUtils/State.h"

rtDeclareVariable(Ray, theRay, rtCurrentRay, );

RT_CALLABLE_PROGRAM void PDF(POptix::Material &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;

	bool sameHemisphere = dot(wiWorld, N) * dot(woWorld, N) > 0 ? true : false;
	prd.pdf = sameHemisphere ? fabsf(dot(wiWorld, N)) * M_1_PIf : 0.0f;			// Importance Sampling
	// prd.pdf = 0.5f * M_1_PI; // (1 / 2PI)									// Uniform Sampling
}

RT_CALLABLE_PROGRAM void Sample(POptix::Material &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)

	float3 dir = UnitSquareToCosineHemisphere(rng2(prd.seed));

	TBN onb(N);
	float3 wo = onb.transform(woWorld);

	if (wo.z < 0.0f)
		dir.z *= -1.0f;

	prd.wi = onb.inverse_transform(dir);
}


RT_CALLABLE_PROGRAM float3 Eval(POptix::Material &mat, State &state, PerRayData &prd)
{
	// https://seblagarde.wordpress.com/2011/08/17/hello-world/
	return mat.albedo * M_1_PIf;
}
