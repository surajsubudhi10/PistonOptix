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

// POptix::Material parameter definition.
rtBuffer<POptix::Material> sysMaterialParameters; // Context global buffer with an array of structures of POptix::Material.
rtDeclareVariable(int, parMaterialIndex, , ); // Per POptix::Material index into the sysMaterialParameters array.
rtDeclareVariable(int, programId, , );

rtBuffer< rtCallableProgramId<void(POptix::Material &mat, State &state, PerRayData &prd)> > sysBRDFPdf;
rtBuffer< rtCallableProgramId<void(POptix::Material &mat, State &state, PerRayData &prd)> > sysBRDFSample;
rtBuffer< rtCallableProgramId<float3(POptix::Material &mat, State &state, PerRayData &prd)> > sysBRDFEval;

rtBuffer< rtCallableProgramId<void(POptix::Light &light, PerRayData &prd, POptix::LightSample &sample, State& state)> > sysLightSample;
rtBuffer<POptix::Light> sysLightParameters;
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

	// Start fresh with the next BSDF sample.  (Either of these values remaining zero is an end-of-path condition.)
	thePrd.radiance = make_float3(0.0f);
	thePrd.f_over_pdf = make_float3(0.0f);
	thePrd.pdf = 0.0f;

	POptix::Material mat = sysMaterialParameters[parMaterialIndex];
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
		sysBRDFSample[POptix::EBrdfTypes::LAMBERT](mat, state, thePrd);
		sysBRDFPdf[POptix::EBrdfTypes::LAMBERT](mat, state, thePrd);
		diffuseBRDF = sysBRDFEval[POptix::EBrdfTypes::LAMBERT](mat, state, thePrd);

		thePrd.brdf_flags |= POptix::BSDF_REFLECTION;
		thePrd.brdf_flags |= POptix::BSDF_DIFFUSE;
	}
	else
	{
		// Specular reflection
		sysBRDFSample[POptix::EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
		sysBRDFPdf[POptix::EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
		specularBRDF = sysBRDFEval[POptix::EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);

		thePrd.brdf_flags |= POptix::BSDF_REFLECTION;
		thePrd.brdf_flags |= (mat.roughness > 0.0f) ? POptix::BSDF_GLOSSY : POptix::BSDF_SPECULAR;
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
	if ((thePrd.brdf_flags & (POptix::BSDF_DIFFUSE | POptix::BSDF_GLOSSY)) && 0 < sysNumberOfLights)
	{
		POptix::LightSample lightSample; // Sample one of many lights.
		lightSample.index = optix::clamp(static_cast<int>(floorf(rng(thePrd.seed) * sysNumberOfLights)), 0, sysNumberOfLights - 1);
		POptix::Light light = sysLightParameters[lightSample.index];
		const POptix::ELightType lightType = light.lightType;

		sysLightSample[lightType](light, thePrd, lightSample, state); // lightSample direction and distance returned in world space!

		rtPrintf("Light Index : %d \n", lightSample.index);
		rtPrintf("Light pdf : %f \n", lightSample.pdf);

		if (lightSample.pdf > 0.0f) // Useful light sample?
		{
			float3 feval = make_float3(0.0f);
			float pdf = 0.0f;

			if (thePrd.brdf_flags & POptix::BSDF_DIFFUSE)
			{
				// Diffuse evaluation
				// Evaluate the Lambert BSDF in the light sample direction. Normally cheaper than shooting rays.
				sysBRDFPdf[POptix::EBrdfTypes::LAMBERT](mat, state, thePrd);
				feval = sysBRDFEval[POptix::EBrdfTypes::LAMBERT](mat, state, thePrd);
				pdf = thePrd.pdf;
			}
			else if (thePrd.brdf_flags & POptix::BSDF_GLOSSY)
			{
				// Specular evaluation
				sysBRDFPdf[POptix::EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
				feval = sysBRDFEval[POptix::EBrdfTypes::MICROFACET_REFLECTION](mat, state, thePrd);
				pdf = thePrd.pdf;
			}

			
			rtPrintf("PDF : %f \n", pdf);

			if (0.0f < pdf && isNotNull(feval))
			{
				// Do the visibility check of the light sample.
				ShadowPRD prdShadow;
				prdShadow.visible = true; // Initialize for miss.

				// Note that the sysSceneEpsilon is applied on both sides of the shadow ray [t_min, t_max] interval 
				// to prevent self intersections with the actual light geometry in the scene!
				rtPrintf("lightSample.direction : %f, %f, %f\n", lightSample.direction.x, lightSample.direction.y, lightSample.direction.z);
				rtPrintf("Light distance : %f \n", lightSample.distance);
				optix::Ray ray = optix::make_Ray(thePrd.hit_pos, lightSample.direction, 1, sysSceneEpsilon, lightSample.distance - sysSceneEpsilon); // Shadow ray.
				rtTrace(sysTopObject, ray, prdShadow);

				if (prdShadow.visible)
				{
					//float NdotL = dot(shading_normal, -lightSample.direction);
					//float lightPdf = lightSample.pdf < 0.0f ? (lightSample.distance * lightSample.distance) / (light.area * NdotL) : lightSample.pdf;

					float lightPdf = lightSample.pdf;
					rtPrintf("Light PDF : %f \n", lightPdf);
					const float misWeight = powerHeuristic(lightPdf, pdf);
					thePrd.radiance += feval * lightSample.emission * (misWeight * fabsf(optix::dot(lightSample.direction, shading_normal)) / lightPdf);
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
