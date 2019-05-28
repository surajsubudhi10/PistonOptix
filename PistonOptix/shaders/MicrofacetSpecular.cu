
#include "per_ray_data.h"
#include "material_parameter.h"
#include "shader_common.h"
#include "PistonOptix/inc/CudaUtils/State.h"

rtDeclareVariable(Ray, theRay, rtCurrentRay, );


RT_FUNCTION float smithG_GGX(float NDotv, float alphaG)
{
	float a = alphaG * alphaG;
	float b = NDotv * NDotv;
	return 1.0f / (NDotv + sqrtf(a + b - a * b));
}

RT_CALLABLE_PROGRAM void PDF(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;
	float3 H = normalize(wiWorld + woWorld);

	float cosTheta = dot(wiWorld, H);
	float sinTheta = sqrtf(1.0f - (cosTheta * cosTheta));
	float alpha = powf(max(0.001f, mat.roughness), 2.0f);
	float alphaSqr = alpha * alpha;
	
	float pdf = alphaSqr * cosTheta * sinTheta / (M_PIf * powf(cosTheta * cosTheta * (alphaSqr - 1.0f) + 1.0f, 2.0f));

	bool sameHemisphere = cosTheta * dot(woWorld, H) > 0 ? true : false;
	prd.pdf = sameHemisphere ? pdf : 0.0f;			// Importance Sampling
}

RT_CALLABLE_PROGRAM void Sample(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	
	float2 r = rng2(prd.seed);

	optix::Onb onb(N); // basis
	float alpha = powf(max(0.001f, mat.roughness), 2.0f);
	float phi = r.x * 2.0f * M_PIf;

	float cosTheta = sqrtf((1.0f - r.y) / (1.0f + (alpha*alpha - 1.0f) * r.y));
	float sinTheta = sqrtf(1.0f - (cosTheta * cosTheta));
	float sinPhi = sinf(phi);
	float cosPhi = cosf(phi);

	float3 half = make_float3(sinTheta*cosPhi, sinTheta*sinPhi, cosTheta);
	onb.inverse_transform(half);
	//AlignVector(N, half);
	float3 dir = 2.0f*dot(woWorld, half)*half - woWorld; //reflection vector

	prd.wi = dir;
}

RT_CALLABLE_PROGRAM float3 Eval(MaterialParameter &mat, State &state, PerRayData &prd)
{
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;
	float3 H = normalize(wiWorld + woWorld);

	float cosTheta = dot(wiWorld, N);
	float alpha = powf(max(0.001f, mat.roughness), 2.0f);
	float alphaSqr = alpha * alpha;

	float3 dielectricSpecular = make_float3(0.04f, 0.04f, 0.04f);
	float3 F0 = lerp(dielectricSpecular, mat.albedo, mat.metallic);
	float3 F = F0 + (1.0f - F0) * powf(1.0f - dot(wiWorld, H), 5.0f);

	float NDotL = dot(N, wiWorld);
	float NDotV = dot(N, woWorld);

	float vis = 0.5f / (NDotL * sqrt(NDotV * NDotV * (1.0f - alphaSqr) + alphaSqr) + NDotV * sqrt(NDotL * NDotL * (1.0f - alphaSqr) + alphaSqr));
	
	float D =  alphaSqr / (M_PIf * powf(cosTheta * cosTheta * (alphaSqr - 1.0f) + 1.0f, 2.0f));

	// https://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v3.pdf (Section 5.6)
	float roughg = powf(mat.roughness*0.5f + 0.5f, 2.0f);
	float G = smithG_GGX(NDotL, roughg) * smithG_GGX(NDotV, roughg);

	return F * G * D;
}
