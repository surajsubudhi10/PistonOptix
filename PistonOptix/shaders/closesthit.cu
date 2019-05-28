#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"
#include "material_parameter.h"
#include "shader_common.h"
#include "PistonOptix/inc/CudaUtils/State.h"
#include "PistonOptix/inc/LightParameters.h"


// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(float, sysSceneEpsilon, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );
rtDeclareVariable(ShadowPRD, prd_shadow, rtPayload, );

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, );
rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

// Material parameter definition.
rtBuffer<MaterialParameter> sysMaterialParameters; // Context global buffer with an array of structures of MaterialParameter.
rtDeclareVariable(int, parMaterialIndex, , ); // Per Material index into the sysMaterialParameters array.
rtDeclareVariable(int, programId, , );

rtBuffer< rtCallableProgramId<void(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFPdf;
rtBuffer< rtCallableProgramId<void(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFSample;
rtBuffer< rtCallableProgramId<float3(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFEval;

rtBuffer< rtCallableProgramId<void(LightParameter &light, PerRayData &prd, LightSample &sample)> > sysLightSample;
rtBuffer<LightParameter> sysLightParameters;

RT_FUNCTION float sdot(float3 x, float3 y)
{
	return clamp(dot(x, y), 0.0f, 1.0f);
}

RT_FUNCTION float SmoothnessToPhongAlpha(float s)
{
	return pow(1000.0f, s * s);
}


RT_PROGRAM void closesthit()
{
	float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal));
	float3 shading_normal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varNormal));

	// Advance the path to the hit position in world coordinates.
	thePrd.hit_pos = theRay.origin + theRay.direction * theIntersectionDistance; 

	// Explicitly include edge-on cases as frontface condition! (Important for nested materials shown in a later example.)
	thePrd.flags |= (0.0f <= optix::dot(thePrd.wo, geoNormal)) ? FLAG_FRONTFACE : 0;

	if ((thePrd.flags & FLAG_FRONTFACE) == 0) // Looking at the backface?
	{
		// Means geometric normal and shading normal are always defined on the side currently looked at.
		// This gives the backfaces of opaque BSDFs a defined result.
		geoNormal = -geoNormal;
		shading_normal = -shading_normal;
	}

	State state;
	state.hit_position = thePrd.hit_pos;
	state.shading_normal = shading_normal;

	// A material system with support for arbitrary mesh lights would evaluate its emission here.
	thePrd.radiance = make_float3(0.0f);

	// Start fresh with the next BSDF sample.  (Either of these values remaining zero is an end-of-path condition.)
	thePrd.f_over_pdf = make_float3(0.0f);
	thePrd.pdf = 0.0f;

	MaterialParameter mat = sysMaterialParameters[parMaterialIndex];

	float3 baseColor = mat.albedo;
	float metallic = mat.metallic;

	float3 diffuseBRDF = make_float3(0.0f);
	float3 specularBRDF = make_float3(0.0f);

	float diffChance = intensity(baseColor);

	// Roulette-select the ray's path
	float roulette = rng(thePrd.seed);
	if (roulette < diffChance) 
	{
		// Diffuse reflection
		sysBRDFSample[EBrdfTypes::LAMBERT](mat, state, thePrd);
		sysBRDFPdf[EBrdfTypes::LAMBERT](mat, state, thePrd);
		diffuseBRDF = sysBRDFEval[EBrdfTypes::LAMBERT](mat, state, thePrd);
	}
	else
	{
		// Specular reflection
		sysBRDFSample[EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
		sysBRDFPdf[EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
		specularBRDF = sysBRDFEval[EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
	}

	float3 wiWorld = thePrd.wi;
	float3 woWorld = -theRay.direction;
	float3 H = normalize(woWorld + wiWorld);

	float3 dielectricSpecular = make_float3(0.04f, 0.04f, 0.04f);
	float3 F0 = lerp(dielectricSpecular, baseColor, metallic);
	float3 F = F0 + (1.0f - F0) * powf(1.0f - dot(wiWorld, H), 5.0f);

	float3 f = (1.0f - F) * diffuseBRDF + specularBRDF;
	

	// Do not sample opaque surfaces below the geometry!
	// Mind that the geometry normal has been flipped to the side the ray points at.
	if (thePrd.pdf <= 0.0f || optix::dot(thePrd.wi, geoNormal) <= 0.0f)
	{
		thePrd.flags |= FLAG_TERMINATE;
		return;
	}

	
	thePrd.f_over_pdf = f * fabsf(optix::dot(thePrd.wi, state.shading_normal)) / thePrd.pdf;

	// Add direct light sample weighted by shadow term and 1/probability.
	// The pdf for a directional area light is 1/solid_angle.

	const LightParameter& light = sysLightParameters[0];
	const float3 light_center = state.hit_position + light.direction;
	const float r1 = rng(thePrd.seed);
	const float r2 = rng(thePrd.seed);
	const float2 disk_sample = square_to_disk(make_float2(r1, r2));
	const float3 jittered_pos = light_center + light.radius*disk_sample.x*light.u + light.radius*disk_sample.y*light.v;
	const float3 L = normalize(jittered_pos - state.hit_position);

	const float NdotL = dot(state.shading_normal, L);
	if (NdotL > 0.0f) 
	{
		ShadowPRD shadow_prd;
		shadow_prd.attenuation = make_float3(1.0f);

		optix::Ray shadow_ray(state.hit_position, L, /*shadow ray type*/ 1, 0.0f);
		rtTrace(sysTopObject, shadow_ray, shadow_prd);

		const float solid_angle = light.radius*light.radius*M_PIf;
		thePrd.radiance += NdotL * light.emission * 0.001f * solid_angle * shadow_prd.attenuation;
	}
}

RT_PROGRAM void any_hit()
{
	prd_shadow.attenuation = make_float3(0.0f);
	rtTerminateRay();
	
	thePrd.flags |= FLAG_TERMINATE;
}
