#include "inc/Application.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include "inc/MyAssert.h"
#include "inc/Scene.h"

namespace POptix
{
	Mesh* Scene::createPlane(const int tessU, const int tessV, const int upAxis)
	{
		MY_ASSERT(1 <= tessU && 1 <= tessV);

		const float uTile = 2.0f / float(tessU);
		const float vTile = 2.0f / float(tessV);

		optix::float3 corner;
		POptix::Mesh* mesh = new POptix::Mesh();

		//std::vector<VertexAttributes> attributes;

		VertexAttributes attrib;

		switch (upAxis)
		{
		case 0: // Positive x-axis is the geometry normal, create geometry on the yz-plane.
			corner = optix::make_float3(0.0f, -1.0f, 1.0f); // Lower front corner of the plane. texcoord (0.0f, 0.0f).

			attrib.tangent = optix::make_float3(0.0f, 0.0f, -1.0f);
			attrib.normal = optix::make_float3(1.0f, 0.0f, 0.0f);

			for (int j = 0; j <= tessV; ++j)
			{
				const float v = float(j) * vTile;

				for (int i = 0; i <= tessU; ++i)
				{
					const float u = float(i) * uTile;

					attrib.vertex = corner + optix::make_float3(0.0f, v, -u);
					attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

					mesh->attributes.push_back(attrib);
				}
			}
			break;

		case 1: // Positive y-axis is the geometry normal, create geometry on the xz-plane.
			corner = optix::make_float3(-1.0f, 0.0f, 1.0f); // left front corner of the plane. texcoord (0.0f, 0.0f).

			attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
			attrib.normal = optix::make_float3(0.0f, 1.0f, 0.0f);

			for (int j = 0; j <= tessV; ++j)
			{
				const float v = float(j) * vTile;

				for (int i = 0; i <= tessU; ++i)
				{
					const float u = float(i) * uTile;

					attrib.vertex = corner + optix::make_float3(u, 0.0f, -v);
					attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

					mesh->attributes.push_back(attrib);
				}
			}
			break;

		case 2: // Positive z-axis is the geometry normal, create geometry on the xy-plane.
			corner = optix::make_float3(-1.0f, -1.0f, 0.0f); // Lower left corner of the plane. texcoord (0.0f, 0.0f).

			attrib.tangent = optix::make_float3(1.0f, 0.0f, 0.0f);
			attrib.normal = optix::make_float3(0.0f, 0.0f, 1.0f);

			for (int j = 0; j <= tessV; ++j)
			{
				const float v = float(j) * vTile;

				for (int i = 0; i <= tessU; ++i)
				{
					const float u = float(i) * uTile;

					attrib.vertex = corner + optix::make_float3(u, v, 0.0f);
					attrib.texcoord = optix::make_float3(u * 0.5f, v * 0.5f, 0.0f);

					mesh->attributes.push_back(attrib);
				}
			}
			break;
		}

		const unsigned int stride = tessU + 1;
		for (int j = 0; j < tessV; ++j)
		{
			for (int i = 0; i < tessU; ++i)
			{
				mesh->indices.push_back(j      * stride + i);
				mesh->indices.push_back(j      * stride + i + 1);
				mesh->indices.push_back((j + 1) * stride + i + 1);

				mesh->indices.push_back((j + 1) * stride + i + 1);
				mesh->indices.push_back((j + 1) * stride + i);
				mesh->indices.push_back(j      * stride + i);
			}
		}

		std::cout << "createPlane(" << upAxis << "): Vertices = " << mesh->attributes.size() << ", Triangles = " << mesh->indices.size() / 3 << std::endl;
		return mesh;
	}

	// Parallelogram from footpoint position, spanned by unnormalized vectors vecU and vecV, normal is normalized and on the CCW frontface.
	Mesh* Scene::createParallelogram(optix::float3 const& position, optix::float3 const& vecU, optix::float3 const& vecV, optix::float3 const& normal)
	{
		Mesh* mesh = new Mesh;
		//std::vector<VertexAttributes> attributes;
		
		VertexAttributes attrib;
		// Same for all four vertices in this parallelogram.
		attrib.tangent = optix::normalize(vecU);
		attrib.normal = normal;

		attrib.vertex = position - 0.5f * (vecU + vecV); // left bottom
		attrib.texcoord = optix::make_float3(0.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = position + 0.5f * (vecU - vecV); // right bottom
		attrib.texcoord = optix::make_float3(1.0f, 0.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = position + 0.5f * (vecU + vecV); // right top
		attrib.texcoord = optix::make_float3(1.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		attrib.vertex = position - 0.5f * (vecU - vecV); // left top
		attrib.texcoord = optix::make_float3(0.0f, 1.0f, 0.0f);
		mesh->attributes.push_back(attrib);

		//std::vector<unsigned int> indices;

		mesh->indices.push_back(0);
		mesh->indices.push_back(1);
		mesh->indices.push_back(2);

		mesh->indices.push_back(2);
		mesh->indices.push_back(3);
		mesh->indices.push_back(0);

		std::cout << "createParallelogram(): Vertices = " << mesh->attributes.size() << ", Triangles = " << mesh->indices.size() / 3 << std::endl;
		return mesh;
	}
}