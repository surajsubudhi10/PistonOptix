#pragma once

#ifndef STATE_H
#define STATE_H


#include <optix.h>
#include <optixu/optixu_math_namespace.h>

struct State
{
	optix::float3 hit_position;
	optix::float3 shading_normal;
};

#endif