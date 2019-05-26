
#include "per_ray_data.h"
#include "material_parameter.h"
#include "shader_common.h"
#include "PistonOptix/inc/CudaUtils/State.h"

rtDeclareVariable(Ray, theRay, rtCurrentRay, );

RT_CALLABLE_PROGRAM void PDF(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;

	float cosTheta = dot(wiWorld, N);
	float alpha = mat.roughness;

	bool sameHemisphere = cosTheta * dot(woWorld, N) > 0 ? true : false;
	prd.pdf = sameHemisphere ? satu(powf(fabsf(cosTheta), alpha)) * M_2_PIf * (alpha + 1.0f) : 0.0f;			// Importance Sampling
}

RT_CALLABLE_PROGRAM void Sample(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)

	float3 wiWorld = reflect(-woWorld, N);
	float3 dir = CosineWeightedHemisphereSampling(rng2(prd.seed), mat.roughness);

	AlignVector(N, dir);

	TBN onb(N);
	prd.wi = /*wiWorld;*/ dir;
	//prd.wi = onb.inverse_transform(dir);
}


RT_CALLABLE_PROGRAM float3 Eval(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;

	float cosTheta = dot(wiWorld, N);
	float alpha = mat.roughness;

	// https://seblagarde.wordpress.com/2011/08/17/hello-world/
	return mat.albedo * M_2_PIf * (alpha + 2.0f) * satu(powf(cosTheta, alpha));
}
