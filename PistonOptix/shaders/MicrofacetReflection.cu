
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

RT_FUNCTION float TrowbridgeReitzDistribution_D(float cosTheta, float alpha)
{
	float Cos2Theta = cosTheta * cosTheta;
	float Sin2Theta = 1.0f - Cos2Theta;
	float tan2Theta = Sin2Theta / Cos2Theta;

	if (tan2Theta > 10e12) 
		return 0.0f;

	const float cos4Theta = Cos2Theta * Cos2Theta;
	float e = (1.0f / (alpha * alpha)) * tan2Theta;
	return 1.0f * charFunc(cosTheta) / (M_PIf * alpha * alpha * cos4Theta * (1 + e) * (1 + e));
}

RT_FUNCTION float TrowbridgeReitzDistribution_Lambda(float cosTheta, float alpha)
{
	float Cos2Theta = cosTheta * cosTheta;
	float SinTheta = sqrtf(1.0f - Cos2Theta);

	float absTanTheta = abs(SinTheta / cosTheta);
	if (isinf(absTanTheta))
		return 0.;

	float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
	return (-1 + sqrt(1.f + alpha2Tan2Theta)) / 2;
}

RT_FUNCTION float TrowbridgeReitzDistribution_G(const float3& vec, const float3& halfVec, const float3& normal, float alpha)
{
	float vDotH = dot(vec, halfVec);
	float vDotN = dot(vec, normal);

	float tan2V = (1.0f - (vDotN * vDotN)) / (vDotN * vDotN);
	float mult = charFunc(vDotH / vDotN);
	float deno = 1.0f + sqrt(1.0f + alpha * alpha * tan2V);

	return mult * 2.0f / deno;
}

RT_FUNCTION float TrowbridgeReitzDistribution_RoughnessToAlpha(float roughness) 
{
	roughness = max(roughness, (float)1e-3);
	float x = log(roughness);
	return 1.62142f + 0.819955f * x + 0.1734f * x * x + 0.0171201f * x * x * x + 0.000640711f * x * x * x * x;
}



RT_CALLABLE_PROGRAM void PDF(MaterialParameter &mat, State &state, PerRayData &prd)
{
	/*
	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;
	float3 H = normalize(wiWorld + woWorld);

	float WoDotH = dot(woWorld, H);
	float cosThetaH = dot(H, N);
	float cosThetaO = (dot(woWorld, N));

	float alpha = TrowbridgeReitzDistribution_RoughnessToAlpha(mat.roughness);
	float D = TrowbridgeReitzDistribution_D(cosThetaH, alpha);
	//float G1 = 1.0f / (1.0f + TrowbridgeReitzDistribution_Lambda(cosThetaO, alpha));

	float pdf = D * G1 * abs(WoDotH) / 4.0f * abs(cosThetaO) * WoDotH;

	bool sameHemisphere = dot(wiWorld, H) * dot(woWorld, H) > 0 ? true : false;
	prd.pdf = sameHemisphere ? pdf : 0.0f;			// Importance Sampling
	*/

	float3 N = state.shading_normal;					// In World Coordinate
	float3 woWorld = -theRay.direction;					// In World Coordinate (viewer direction)
	float3 wiWorld = prd.wi;
	float3 H = normalize(wiWorld + woWorld);

	float cosTheta = dot(wiWorld, H);
	float sinTheta = sqrtf(1.0f - (cosTheta * cosTheta));
	
	float alpha = powf(max(0.001f, mat.roughness), 2.0f);
	float D = TrowbridgeReitzDistribution_D(cosTheta, alpha);

	float pdf = D * cosTheta * sinTheta;

	bool sameHemisphere = dot(wiWorld, H) * dot(woWorld, H) > 0 ? true : false;
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

	float cosThetaO = fabsf(dot(woWorld, N));
	float cosThetaI = fabsf(dot(wiWorld, N));

	if (cosThetaI <= 0.0f || cosThetaO <= 0.0f) 
	{
		prd.flags |= FLAG_TERMINATE;
		return make_float3(0.0f);
	}

	if (H.x == 0 && H.y == 0 && H.z == 0) 
	{
		prd.flags |= FLAG_TERMINATE;
		return make_float3(0.0f);
	}

	float3 dielectricSpecular = make_float3(0.04f, 0.04f, 0.04f);
	float3 F0 = lerp(dielectricSpecular, mat.albedo, mat.metallic);
	float3 F = F0 + (1.0f - F0) * powf(1.0f - dot(wiWorld, H), 5.0f);

	float cosThetaH = dot(H, N);

	float alpha = powf(max(0.001f, mat.roughness), 2.0f);
	float D = TrowbridgeReitzDistribution_D(cosThetaI, alpha);
	float G = TrowbridgeReitzDistribution_G(woWorld, H, N, alpha) * TrowbridgeReitzDistribution_G(wiWorld, H, N, alpha);

	return F * G * D;// / (4.0f * fabsf(cosThetaI) * fabsf(cosThetaO));
}
