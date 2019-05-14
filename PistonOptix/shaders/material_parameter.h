#pragma once

#ifndef MATERIAL_PARAMETER_H
#define MATERIAL_PARAMETER_H

// Just some hardcoded material parameter system.
struct MaterialParameter
{
	optix::float3 albedo;     // albedo, color, tint, throughput change for specular surfaces. Pick your meaning.

	// Manual padding to 16-byte alignment goes here.
	float unused0;
};

#endif // MATERIAL_PARAMETER_H
