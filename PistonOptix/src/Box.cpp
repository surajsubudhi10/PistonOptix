#include <cstring>
#include <iostream>
#include <sstream>

#include "inc/Scene.h"

namespace POptix
{
	// A simple unit cube built from 12 triangles.
	Mesh* Scene::createBox()
	{
		float left = -1.0f;
		float right = 1.0f;
		float bottom = -1.0f;
		float top = 1.0f;
		float back = -1.0f;
		float front = 1.0f;

		POptix::Mesh* mesh = new POptix::Mesh;

		VertexAttributes attrib;
		// Left.
		attrib.tangent = optix::make_float3(0.0f, 0.0f, 1.0f);
		attrib.normal = optix::make_float3(-1.0f, 0.0f, 0.0f);

		attrib.vertex = optix::make_float3(left, bottom, back);
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, bottom, front);
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, top, front);
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, top, back);
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		// Right.
		attrib.tangent = optix::make_float3(0.0f, 0.0f, -1.0f);
		attrib.normal = optix::make_float3(1.0f, 0.0f, 0.0f);

		attrib.vertex = optix::make_float3(right, bottom, front);
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, bottom, back);
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, top, back);
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, top, front);
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		// Back.  
		attrib.tangent = optix::make_float3(-1.0f, 0.0f, 0.0f);
		attrib.normal = optix::make_float3(0.0f, 0.0f, -1.0f);

		attrib.vertex = optix::make_float3(right, bottom, back);
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, bottom, back);
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, top, back);
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, top, back);
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		// Front.
		attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
		attrib.normal = optix::make_float3(0.0f, 0.0f, 1.0f);

		attrib.vertex = optix::make_float3(left, bottom, front);
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, bottom, front);
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, top, front);
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, top, front);
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		// Bottom.
		attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
		attrib.normal = optix::make_float3(0.0f, -1.0f, 0.0f);

		attrib.vertex = optix::make_float3(left, bottom, back);
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, bottom, back);
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, bottom, front);
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, bottom, front);
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		// Top.
		attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
		attrib.normal = optix::make_float3(0.0f, 1.0f, 0.0f);

		attrib.vertex = optix::make_float3(left, top, front);
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, top, front);
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(right, top, back);
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = optix::make_float3(left, top, back);
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);


		for (unsigned int i = 0; i < 6; ++i) // Six faces (== 12 triangles).
		{
			const unsigned int idx = i * 4; // Four unique attributes per box face.

			mesh->indices.push_back(idx);
			mesh->indices.push_back(idx + 1);
			mesh->indices.push_back(idx + 2);

			mesh->indices.push_back(idx + 2);
			mesh->indices.push_back(idx + 3);
			mesh->indices.push_back(idx);
		}

		std::cout << "createBox(): Vertices = " << mesh->attributes.size() << ", Triangles = " << mesh->indices.size() / 3 << std::endl;
		return mesh;
	}
}