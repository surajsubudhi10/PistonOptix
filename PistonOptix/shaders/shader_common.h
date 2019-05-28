#pragma once

#ifndef SHADER_COMMON_H
#define SHADER_COMMON_H

#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "rt_function.h"

using namespace optix;

// Explicitly not named Onb to not conflict with the optix::Onb
// Tangent-Bitangent-Normal orthonormal space.
struct TBN
{
	// Default constructor to be able to include it into other structures when needed.
	RT_FUNCTION TBN()
	{
	}

	RT_FUNCTION TBN(const optix::float3& n)
		: normal(n)
	{
		if (fabs(normal.x) > fabs(normal.z))
		{
			bitangent.x = -normal.y;
			bitangent.y = normal.x;
			bitangent.z = 0;
		}
		else
		{
			bitangent.x = 0;
			bitangent.y = -normal.z;
			bitangent.z = normal.y;
		}

		bitangent = normalize(bitangent);
		tangent = cross(bitangent, normal);
	}

	// Constructor for cases where tangent, bitangent, and normal are given as ortho-normal basis.
	RT_FUNCTION TBN(const optix::float3& t, const optix::float3& b, const optix::float3& n)
		: tangent(t)
		, bitangent(b)
		, normal(n)
	{
	}

	// Normal is kept, tangent and bitangent are calculated.
	// Normal must be normalized.
	// Must not be used with degenerated vectors!
	RT_FUNCTION TBN(const optix::float3& tangent_reference, const optix::float3& n)
		: normal(n)
	{
		bitangent = optix::normalize(optix::cross(normal, tangent_reference));
		tangent = optix::cross(bitangent, normal);
	}

	RT_FUNCTION void negate()
	{
		tangent = -tangent;
		bitangent = -bitangent;
		normal = -normal;
	}

	RT_FUNCTION optix::float3 transform(const optix::float3& p) const
	{
		return optix::make_float3(optix::dot(p, tangent),
			optix::dot(p, bitangent),
			optix::dot(p, normal));
	}

	RT_FUNCTION optix::float3 inverse_transform(const optix::float3& p) const
	{
		return p.x * tangent + p.y * bitangent + p.z * normal;
	}

	optix::float3 tangent;
	optix::float3 bitangent;
	optix::float3 normal;
};


RT_FUNCTION float luminance(const optix::float3& rgb)
{
  const optix::float3 ntsc_luminance = { 0.30f, 0.59f, 0.11f };
  return optix::dot(rgb, ntsc_luminance);
}

RT_FUNCTION float intensity(const optix::float3& rgb)
{
	return (rgb.x + rgb.y + rgb.z) * 0.3333333333f;
}

RT_FUNCTION float intensity3(const optix::float4& rgb)
{
	return (rgb.x + rgb.y + rgb.z) * 0.3333333333f;
}

RT_FUNCTION float cube(const float x)
{
	return x * x * x;
}

// Helper functions.
RT_FUNCTION optix::float3 logf(const optix::float3& v)
{
	return optix::make_float3(::logf(v.x), ::logf(v.y), ::logf(v.z));
}

RT_FUNCTION optix::float2 floorf(const optix::float2& v)
{
	return optix::make_float2(::floorf(v.x), ::floorf(v.y));
}

RT_FUNCTION optix::float3 floorf(const optix::float3& v)
{
	return optix::make_float3(::floorf(v.x), ::floorf(v.y), ::floorf(v.z));
}

RT_FUNCTION optix::float3 ceilf(const optix::float3& v)
{
	return optix::make_float3(::ceilf(v.x), ::ceilf(v.y), ::ceilf(v.z));
}

RT_FUNCTION optix::float3 powf(const optix::float3& v, const float e)
{
	return optix::make_float3(::powf(v.x, e), ::powf(v.y, e), ::powf(v.z, e));
}

RT_FUNCTION optix::float4 powf(const optix::float4& v, const float e)
{
	return optix::make_float4(::powf(v.x, e), ::powf(v.y, e), ::powf(v.z, e), ::powf(v.w, e));
}


RT_FUNCTION optix::float2 fminf(const optix::float2& v, const float m)
{
	return optix::make_float2(::fminf(v.x, m), ::fminf(v.y, m));
}
RT_FUNCTION optix::float3 fminf(const optix::float3& v, const float m)
{
	return optix::make_float3(::fminf(v.x, m), ::fminf(v.y, m), ::fminf(v.z, m));
}
RT_FUNCTION optix::float4 fminf(const optix::float4& v, const float m)
{
	return optix::make_float4(::fminf(v.x, m), ::fminf(v.y, m), ::fminf(v.z, m), ::fminf(v.w, m));
}

RT_FUNCTION optix::float2 fminf(const float m, const optix::float2& v)
{
	return optix::make_float2(::fminf(m, v.x), ::fminf(m, v.y));
}
RT_FUNCTION optix::float3 fminf(const float m, const optix::float3& v)
{
	return optix::make_float3(::fminf(m, v.x), ::fminf(m, v.y), ::fminf(m, v.z));
}
RT_FUNCTION optix::float4 fminf(const float m, const optix::float4& v)
{
	return optix::make_float4(::fminf(m, v.x), ::fminf(m, v.y), ::fminf(m, v.z), ::fminf(m, v.w));
}


RT_FUNCTION optix::float2 fmaxf(const optix::float2& v, const float m)
{
	return optix::make_float2(::fmaxf(v.x, m), ::fmaxf(v.y, m));
}
RT_FUNCTION optix::float3 fmaxf(const optix::float3& v, const float m)
{
	return optix::make_float3(::fmaxf(v.x, m), ::fmaxf(v.y, m), ::fmaxf(v.z, m));
}
RT_FUNCTION optix::float4 fmaxf(const optix::float4& v, const float m)
{
	return optix::make_float4(::fmaxf(v.x, m), ::fmaxf(v.y, m), ::fmaxf(v.z, m), ::fmaxf(v.w, m));
}

RT_FUNCTION optix::float2 fmaxf(const float m, const optix::float2& v)
{
	return optix::make_float2(::fmaxf(m, v.x), ::fmaxf(m, v.y));
}
RT_FUNCTION optix::float3 fmaxf(const float m, const optix::float3& v)
{
	return optix::make_float3(::fmaxf(m, v.x), ::fmaxf(m, v.y), ::fmaxf(m, v.z));
}
RT_FUNCTION optix::float4 fmaxf(const float m, const optix::float4& v)
{
	return optix::make_float4(::fmaxf(m, v.x), ::fmaxf(m, v.y), ::fmaxf(m, v.z), ::fmaxf(m, v.w));
}

RT_FUNCTION optix::float3 fpowf(const optix::float3& u, const optix::float3& v)
{
	return make_float3(powf(u.x, v.x), powf(u.y, v.y), powf(u.z, v.z));
}

RT_FUNCTION float satu(float val) 
{
	return clamp(val, 0.0f, 1.0f);
}

RT_FUNCTION bool isNull(const optix::float3& v)
{
	return (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f);
}

RT_FUNCTION bool isNotNull(const optix::float3& v)
{
	return (v.x != 0.0f || v.y != 0.0f || v.z != 0.0f);
}

RT_FUNCTION float charFunc(float val) 
{
	return val > 0 ? 1.0f : 0.0f;
}

// Used for Multiple Importance Sampling.
RT_FUNCTION float powerHeuristic(const float a, const float b)
{
	const float t = a * a;
	return t / (t + b * b);
}

RT_FUNCTION float balanceHeuristic(const float a, const float b)
{
	return a / (a + b);
}


RT_FUNCTION void AlignVector(float3 const& axis, float3& w)
{
	// Align w with axis.
	const float s = copysign(1.0f, axis.z);
	w.z *= s;
	const float3 h = make_float3(axis.x, axis.y, axis.z + s);
	const float  k = optix::dot(w, h) / (1.0f + fabsf(axis.z));
	w = k * h - w;
}


// Sampling
RT_FUNCTION float3 UniformHemisphereSampling(float2 point) 
{
	float cosTheta = point.x;
	float sinTheta = sqrtf(fmaxf(0.0f, 1.0f - cosTheta * cosTheta));

	float phi = 2 * M_PIf * point.y;
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);

	return make_float3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}

RT_FUNCTION float3 UnitSquareToCosineHemisphere(const float2 sample)
{
	// Choose a point on the local hemisphere coordinates about +z.
	const float theta = 2.0f * M_PIf * sample.x;
	const float r = sqrtf(sample.y);
	float x = r * cosf(theta);
	float y = r * sinf(theta);
	float z = 1.0f - x * x - y * y;
	z = (0.0f < z) ? sqrtf(z) : 0.0f;

	return make_float3(x, y, z);
}

RT_FUNCTION float3 CosineWeightedHemisphereSampling(float2 sample, float alpha) 
{
	const float cosTheta = powf((1.0f - sample.y), 1.0f / (alpha + 1.0f));
	const float sinTheta = sqrtf(fmaxf(0.0f, 1.0f - cosTheta * cosTheta));

	float phi = 2 * M_PIf * sample.y;
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);

	return make_float3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}

#endif // SHADER_COMMON_H
