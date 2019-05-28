#pragma once

#ifndef MATERIAL_PARAMETER_H
#define MATERIAL_PARAMETER_H

enum EBrdfTypes 
{
	LAMBERT,
	PHONG,
	MICROFACET_REFLECTION,

	NUM_OF_BRDF
};


// Just some hardcoded material parameter system.
struct MaterialParameter
{
	optix::float3 albedo;     // albedo, color, tint, throughput change for specular surfaces. Pick your meaning.
	float metallic;
	float roughness;
};

#endif // MATERIAL_PARAMETER_H
