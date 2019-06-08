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
		optix::float3 normal;	// for directional light its the direction of light
		optix::float3 emission;
		optix::float3 u;
		optix::float3 v;
		float area;
		float radius;
		bool isDelta;
		ELightType lightType;
	};

	struct LightSample
	{
		optix::float3 surfacePos;
		int           index;
		optix::float3 direction;	// Direction is from the hit point to the light surface (wi)
		float         distance;
		optix::float3 emission;
		float         pdf;
	};
}
#endif // LIGHT_PARAMETERS_H
