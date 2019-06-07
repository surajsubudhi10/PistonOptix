#pragma once

#ifndef LIGHT_PARAMETERS_H
#define LIGHT_PARAMETERS_H

#include "PistonOptix\shaders\shader_common.h"
#include "PistonOptix\shaders\rt_function.h"

namespace POptix
{
	enum ELightType
	{
		SPHERE,
		QUAD,
		DIRECTIONAL,

		NUM_OF_LIGHT_TYPE
	};

	struct Light
	{
		optix::float3 position;
		optix::float3 direction;	// for area light its the normal direction
		optix::float3 emission;
		optix::float3 u;
		optix::float3 v;
		float area;
		float radius;
		ELightType lightType;
	};

	struct LightSample
	{
		optix::float3 surfacePos;
		int           index;
		optix::float3 direction;
		float         distance;
		optix::float3 emission;
		float         pdf;
	};
}
#endif // LIGHT_PARAMETERS_H
