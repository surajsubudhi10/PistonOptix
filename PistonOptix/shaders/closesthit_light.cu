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

// Semantic variables.
rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, theIntersectionDistance, rtIntersectionDistance, );

rtDeclareVariable(PerRayData, thePrd, rtPayload, );

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );

rtBuffer<POptix::Light> sysLightParameters;
rtDeclareVariable(int, parMaterialIndex, , );  // Index into the sysLightDefinitions array.
rtDeclareVariable(int, sysNumberOfLights, , );


RT_PROGRAM void closesthit_light()
{
	thePrd.hit_pos = theRay.origin + theRay.direction * theIntersectionDistance;

	// PERF Not really needed when it's know that light geometry is not under Transforms.
	const float3 geoNormal = optix::normalize(rtTransformNormal(RT_OBJECT_TO_WORLD, varGeoNormal));

	const float cosTheta = optix::dot(thePrd.wo, geoNormal);
	thePrd.flags |= (0.0f <= cosTheta) ? FLAG_FRONTFACE : 0;

	thePrd.radiance = make_float3(0.0f); // Backside is black.

	if (thePrd.flags & FLAG_FRONTFACE) // Looking at the front face?
	{
		const POptix::Light light = sysLightParameters[parMaterialIndex];
		thePrd.radiance = light.emission;

#if USE_NEXT_EVENT_ESTIMATION
		const float pdfLight = (theIntersectionDistance * theIntersectionDistance) / (light.area * cosTheta);
		// If it's an implicit light hit from a diffuse scattering event and the light emission was not returning a zero pdf.
		if ((thePrd.brdf_flags & (POptix::BSDF_DIFFUSE | POptix::BSDF_GLOSSY)) && DENOMINATOR_EPSILON < pdfLight)
		{
			// Scale the emission with the power heuristic between the previous BSDF sample pdf and this implicit light sample pdf.
			thePrd.radiance *= powerHeuristic(thePrd.pdf, pdfLight);
		}
#endif // USE_NEXT_EVENT_ESTIMATION
	}

	// Lights have no other material properties than emission in this demo. Terminate the path.
	thePrd.flags |= FLAG_TERMINATE;
}
