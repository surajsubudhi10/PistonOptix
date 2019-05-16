#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>

#include "rt_function.h"
#include "per_ray_data.h"
#include "material_parameter.h"
#include "shader_common.h"
#include "PistonOptix/inc/CudaUtils/State.h"

// Context global variables provided by the renderer system.
rtDeclareVariable(rtObject, sysTopObject, , );
rtDeclareVariable(float, sysSceneEpsilon, , );

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
//rtDeclareVariable(optix::float3, varTangent,   attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, );
//rtDeclareVariable(optix::float3, varTexCoord,  attribute TEXCOORD, ); 

// Material parameter definition.
rtBuffer<MaterialParameter> sysMaterialParameters; // Context global buffer with an array of structures of MaterialParameter.
rtDeclareVariable(int, parMaterialIndex, , ); // Per Material index into the sysMaterialParameters array.
rtDeclareVariable(int, programId, , );

rtBuffer< rtCallableProgramId<void(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFPdf;
rtBuffer< rtCallableProgramId<void(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFSample;
rtBuffer< rtCallableProgramId<float3(MaterialParameter &mat, State &state, PerRayData &prd)> > sysBRDFEval;


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
		// Do not recalculate the frontface condition!
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


	float3 f = make_float3(0.0f, 0.0f, 0.0f);
	mat.albedo = fminf(1.0f - mat.specular, mat.albedo);
	float specChance = intensity(mat.specular);
	float diffChance = intensity(mat.albedo);

	// Roulette-select the ray's path
	float roulette = rng(thePrd.seed);
	if (roulette < diffChance)
	{
		// Diffuse reflection
		sysBRDFSample[0](mat, state, thePrd);
		sysBRDFPdf[0](mat, state, thePrd);
		f = sysBRDFEval[0](mat, state, thePrd) / diffChance;
	}
	else if(roulette < specChance + diffChance)
	{
		mat.roughness = SmoothnessToPhongAlpha(1 - mat.roughness);

		// Specular reflection
		sysBRDFSample[1](mat, state, thePrd);
		sysBRDFPdf[1](mat, state, thePrd);
		f = sysBRDFEval[1](mat, state, thePrd) / specChance;
	}

	// Do not sample opaque surfaces below the geometry!
	// Mind that the geometry normal has been flipped to the side the ray points at.
	if (thePrd.pdf <= 0.0f || optix::dot(thePrd.wi, geoNormal) <= 0.0f)
	{
		thePrd.flags |= FLAG_TERMINATE;
		return;
	}

	
	thePrd.f_over_pdf = f * fabsf(optix::dot(thePrd.wi, state.shading_normal)) / thePrd.pdf;
}
