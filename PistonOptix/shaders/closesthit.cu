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
rtDeclareVariable(optix::float3, varTangent, attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, );
rtDeclareVariable(optix::float3, varTexCoord, attribute TEXCOORD, );

// Material parameter definition.
rtBuffer<MaterialParameter> sysMaterialParameters; // Context global buffer with an array of structures of MaterialParameter.
rtDeclareVariable(int, parMaterialIndex, , ); // Per Material index into the sysMaterialParameters array.
rtDeclareVariable(int, programId, , );

rtBuffer< rtCallableProgramId<void(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFPdf;
rtBuffer< rtCallableProgramId<void(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFSample;
rtBuffer< rtCallableProgramId<float3(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFEval;

rtBuffer< rtCallableProgramId<void(LightParameter &light, PerRayData &prd, LightSample &sample)> > sysLightSample;
rtBuffer<LightParameter> sysLightParameters;
rtDeclareVariable(int, sysNumberOfLights, , );

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
	state.geometry_normal = geoNormal;

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

	// Roulette-select the ray's path
	float roulette = rng(thePrd.seed);
	float diffChance = 0.5f * (1.0f - mat.metallic);
	if (roulette < diffChance)
	{
		// Diffuse reflection
		sysBRDFSample[EBrdfTypes::LAMBERT](mat, state, thePrd);
		sysBRDFPdf[EBrdfTypes::LAMBERT](mat, state, thePrd);
		diffuseBRDF = sysBRDFEval[EBrdfTypes::LAMBERT](mat, state, thePrd);

		thePrd.brdf_flags |= BSDF_REFLECTION;
		thePrd.brdf_flags |= BSDF_DIFFUSE;
	}
	else
	{
		// Specular reflection
		sysBRDFSample[EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
		sysBRDFPdf[EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
		specularBRDF = sysBRDFEval[EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);

		thePrd.brdf_flags |= BSDF_REFLECTION;
		thePrd.brdf_flags |= (mat.roughness > 0.0f) ? BSDF_GLOSSY : BSDF_SPECULAR;
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

#if USE_NEXT_EVENT_ESTIMATION
	// Direct lighting if the sampled BSDF was diffuse and any light is in the scene.

	if ((thePrd.brdf_flags & (BSDF_DIFFUSE /*| BSDF_GLOSSY*/)) && 0 < sysNumberOfLights)
	{
		LightSample lightSample; // Sample one of many lights.
		lightSample.index = optix::clamp(static_cast<int>(floorf(rng(thePrd.seed) * sysNumberOfLights)), 0, sysNumberOfLights - 1);
		LightParameter light = sysLightParameters[lightSample.index];
		const ELightType lightType = light.lightType;
		lightSample.distance = RT_DEFAULT_MAX;

		sysLightSample[lightType](light, thePrd, lightSample); // lightSample direction and distance returned in world space!

		if (0.0f < lightSample.pdf) // Useful light sample?
		{
			// Lambert evaluation
			// Evaluate the Lambert BSDF in the light sample direction. Normally cheaper than shooting rays.
			sysBRDFPdf[EBrdfTypes::LAMBERT](mat, state, thePrd);
			const float3 f = sysBRDFEval[EBrdfTypes::LAMBERT](mat, state, thePrd);
			const float  pdf = thePrd.pdf;

			if (0.0f < pdf && isNotNull(f))
			{
				// Do the visibility check of the light sample.
				ShadowPRD prdShadow;
				prdShadow.visible = true; // Initialize for miss.

				// Note that the sysSceneEpsilon is applied on both sides of the shadow ray [t_min, t_max] interval 
				// to prevent self intersections with the actual light geometry in the scene!
				optix::Ray ray = optix::make_Ray(thePrd.hit_pos, lightSample.direction, 1, sysSceneEpsilon, lightSample.distance - sysSceneEpsilon); // Shadow ray.
				rtTrace(sysTopObject, ray, prdShadow);

				if (prdShadow.visible)
				{
					const float misWeight = powerHeuristic(lightSample.pdf, pdf);
					thePrd.radiance += f * lightSample.emission * (misWeight * optix::dot(lightSample.direction, shading_normal) / lightSample.pdf);
				}
			}
		}
	}
#endif // USE_NEXT_EVENT_ESTIMATION
}

RT_PROGRAM void any_hit()
{
	prd_shadow.visible = false;
	rtTerminateRay();

	thePrd.flags |= FLAG_TERMINATE;
}
