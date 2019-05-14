#pragma once

#ifndef SHADER_COMMON_H
#define SHADER_COMMON_H

#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include <optixu/optixu_matrix_namespace.h>

#include "rt_function.h"

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
		if (fabsf(normal.z) < fabsf(normal.x))
		{
			tangent.x = normal.z;
			tangent.y = 0.0f;
			tangent.z = -normal.x;
		}
		else
		{
			tangent.x = 0.0f;
			tangent.y = normal.z;
			tangent.z = -normal.y;
		}
		tangent = optix::normalize(tangent);
		bitangent = optix::cross(normal, tangent);
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


RT_FUNCTION bool isNull(const optix::float3& v)
{
	return (v.x == 0.0f && v.y == 0.0f && v.z == 0.0f);
}

RT_FUNCTION bool isNotNull(const optix::float3& v)
{
	return (v.x != 0.0f || v.y != 0.0f || v.z != 0.0f);
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


/**
* Orthonormal basis
*/
struct OrthoNormBasis
{
	__forceinline__ RT_HOSTDEVICE OrthoNormBasis(const float3& normal)
	{
		m_normal = normalize(normal);

		if (dot(m_normal, make_float3(0.0f, 1.0f, 0.0f)) != 1.0f)
		{
			m_tangent = normalize(cross(m_normal, make_float3(0.0f, 1.0f, 0.0f)));
		}
		else
		{
			m_tangent = make_float3(1.0f, 0.0f, 0.0f);
		}

		m_binormal = cross(m_tangent, m_normal);


		onbMat.setRow(0, m_tangent);
		onbMat.setRow(1, m_binormal);
		onbMat.setRow(2, m_normal);
	}

	__forceinline__ RT_HOSTDEVICE void ToBasisCoordinate(float3& p) const
	{
		p = onbMat * p;
		//p = make_float3(dot(p, m_tangent), dot(p, m_binormal), dot(p, m_normal));
	}

	__forceinline__ RT_HOSTDEVICE void ToWorldCoordinate(float3& p) const
	{
		p = onbMat.transpose() * p;
	}

	// left hand coordinate frame
	float3 m_tangent;    // x // u
	float3 m_binormal;   // y // v
	float3 m_normal;     // z // w

	Matrix3x3 onbMat;
};


// Sampling
RT_FUNCTION float3 UniformHemisphereSampling(float2 point) 
{
	float cosTheta = point.x;
	float sinTheta = sqrtf(max(0.0f, 1.0f - cosTheta * cosTheta));

	float phi = 2 * M_PI * point.y;
	float cosPhi = cosf(phi);
	float sinPhi = sinf(phi);

	return make_float3(sinTheta * cosPhi, sinTheta * sinPhi, cosTheta);
}



#endif // SHADER_COMMON_H
