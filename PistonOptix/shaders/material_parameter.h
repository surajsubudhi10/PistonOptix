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

enum BRDFFlags 
{
	BSDF_REFLECTION		= 1 << 0,
	BSDF_TRANSMISSION	= 1 << 1,
	BSDF_DIFFUSE		= 1 << 2,
	BSDF_GLOSSY			= 1 << 3,
	BSDF_SPECULAR		= 1 << 4,

	BSDF_ALL = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR | BSDF_REFLECTION |
	BSDF_TRANSMISSION,
};


// Just some hardcoded material parameter system.
struct MaterialParameter
{
	optix::float3 albedo;     // albedo, color, tint, throughput change for specular surfaces. Pick your meaning.
	float metallic;
	float roughness;
};

#endif // MATERIAL_PARAMETER_H
