#pragma once

#ifndef VERTEX_ATTRIBUTES_H
#define VERTEX_ATTRIBUTES_H

struct VertexAttributes
{
	optix::float3 vertex;
	optix::float3 tangent;
	optix::float3 normal;
	optix::float3 texcoord;
};

#endif // VERTEX_ATTRIBUTES_H
