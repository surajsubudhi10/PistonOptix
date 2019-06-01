#include "app_config.h"

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "rt_function.h"
#include "vertex_attributes.h"

rtBuffer<VertexAttributes> attributesBuffer;
rtBuffer<uint3>            indicesBuffer;

// Attributes.
rtDeclareVariable(optix::float3, varGeoNormal, attribute GEO_NORMAL, );
rtDeclareVariable(optix::float3, varTangent, attribute TANGENT, );
rtDeclareVariable(optix::float3, varNormal, attribute NORMAL, );
rtDeclareVariable(optix::float3, varTexCoord, attribute TEXCOORD, );

rtDeclareVariable(optix::Ray, theRay, rtCurrentRay, );
rtDeclareVariable(float, sysSceneEpsilon, , );

using namespace optix;

RT_FUNCTION bool IsInsideTriangle(const float3& v0, const float3& v1, const float3& v2, const float3& P)
{
	float3 edge0 = v1 - v0;
	float3 edge1 = v2 - v1;
	float3 edge2 = v0 - v2;
	float3 N = cross(-edge2, edge0);

	float3 C0 = P - v0;
	float3 C1 = P - v1;
	float3 C2 = P - v2;
	if (dot(N, cross(edge0, C0)) > 0 &&
		dot(N, cross(edge1, C1)) > 0 &&
		dot(N, cross(edge2, C2)) > 0) return true; // P is inside the triangle 

	return false;
}

RT_FUNCTION bool IntersectTriangle(const Ray& ray, const float3& v0, const float3& v1, const float3& v2, float3& N, float& t, float& beta, float& gamma) 
{
	// compute plane's normal
	const float3 ed0 = v1 - v0;
	const float3 ed1 = v0 - v2;
	N = cross(ed1, ed0);

	double a = v0.x - v1.x, b = v0.x - v2.x, c = ray.direction.x, d = v0.x - ray.origin.x;
	double e = v0.y - v1.y, f = v0.y - v2.y, g = ray.direction.y, h = v0.y - ray.origin.y;
	double i = v0.z - v1.z, j = v0.z - v2.z, k = ray.direction.z, l = v0.z - ray.origin.z;

	double m = f * k - g * j, n = h * k - g * l, p = f * l - h * j;
	double q = g * i - e * k, s = e * j - f * i;
	double inv_denom = 1.0 / (a * m + b * q + c * s);
	double e1 = d * m - b * n - c * p;
	
	beta = e1 * inv_denom;
	if (beta < 0.0)
		return (false);
	
	double r = r = e * l - h * i;
	double e2 = a * n + d * q + c * r;
	gamma = e2 * inv_denom;
	if (gamma < 0.0)
		return (false);
	if (beta + gamma > 1.0)
		return false;
	
	double e3 = a * p - b * r + d * s;
	t = e3 * inv_denom;
	if (t < ray.tmin && t > ray.tmax)
		return false;
	
	return true;
}

// Intersection routine for indexed interleaved triangle data.
RT_PROGRAM void intersection_triangle_indexed(int primitiveIndex)
{
	const uint3 indices = indicesBuffer[primitiveIndex];

	VertexAttributes const& a0 = attributesBuffer[indices.x];
	VertexAttributes const& a1 = attributesBuffer[indices.y];
	VertexAttributes const& a2 = attributesBuffer[indices.z];

	const float3 v0 = a0.vertex;
	const float3 v1 = a1.vertex;
	const float3 v2 = a2.vertex;

	float3 n;
	float  t;
	float  beta;
	float  gamma;

	//if (intersect_triangle(theRay, v0, v1, v2, n, t, beta, gamma))
	if (IntersectTriangle(theRay, v0, v1, v2, n, t, beta, gamma))
	{
		if (rtPotentialIntersection(t))
		{
			// Barycentric interpolation:
			const float alpha = 1.0f - beta - gamma;

			// Note: No normalization on the TBN attributes here for performance reasons.
			//       It's done after the transformation into world space anyway.
			varGeoNormal = n;
			varTangent = a0.tangent  * alpha + a1.tangent  * beta + a2.tangent  * gamma;
			varNormal = a0.normal   * alpha + a1.normal   * beta + a2.normal   * gamma;
			varTexCoord = a0.texcoord * alpha + a1.texcoord * beta + a2.texcoord * gamma;

			rtReportIntersection(0);
		}
	}
}
