#include "inc/Application.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include "inc/MyAssert.h"
#include "inc/Scene.h"

namespace POptix
{
	/*
	Mesh* Scene::createSphere(const int tessU, const int tessV, const float radius, const float maxTheta)
	{
		MY_ASSERT(3 <= tessU && 3 <= tessV);

		POptix::Mesh* mesh = new POptix::Mesh;

		mesh->attributes.reserve((tessU + 1) * tessV);
		mesh->indices.reserve(6 * tessU * (tessV - 1));

		float phi_step = 2.0f * M_PIf / (float)tessU;
		float theta_step = maxTheta / (float)(tessV - 1);

		// Latitudinal rings.
		// Starting at the south pole going upwards on the y-axis.
		for (int latitude = 0; latitude < tessV; latitude++) // theta angle
		{
			float theta = (float)latitude * theta_step;
			float sinTheta = sinf(theta);
			float cosTheta = cosf(theta);

			float texv = (float)latitude / (float)(tessV - 1); // Range [0.0f, 1.0f]

			// Generate vertices along the latitudinal rings.
			// On each latitude there are tessU + 1 vertices.
			// The last one and the first one are on identical positions, but have different texture coordinates!
			// DAR FIXME Note that each second triangle connected to the two poles has zero area!
			for (int longitude = 0; longitude <= tessU; longitude++) // phi angle
			{
				float phi = (float)longitude * phi_step;
				float sinPhi = sinf(phi);
				float cosPhi = cosf(phi);

				float texu = (float)longitude / (float)tessU; // Range [0.0f, 1.0f]

				// Unit sphere coordinates are the normals.
				optix::float3 normal = optix::make_float3(cosPhi * sinTheta,
					-cosTheta,                 // -y to start at the south pole.
					-sinPhi * sinTheta);
				VertexAttributes attrib;

				attrib.vertex = normal * radius;
				attrib.tangent = optix::make_float3(-sinPhi, 0.0f, -cosPhi);
				attrib.normal = normal;
				attrib.texcoord = optix::make_float3(texu, texv, 0.0f);

				mesh->attributes.push_back(attrib);
			}
		}

		// We have generated tessU + 1 vertices per latitude.
		const unsigned int columns = tessU + 1;

		// Calculate indices.
		for (int latitude = 0; latitude < tessV - 1; latitude++)
		{
			for (int longitude = 0; longitude < tessU; longitude++)
			{
				mesh->indices.push_back(latitude      * columns + longitude);  // lower left
				mesh->indices.push_back(latitude      * columns + longitude + 1);  // lower right
				mesh->indices.push_back((latitude + 1) * columns + longitude + 1);  // upper right 

				mesh->indices.push_back((latitude + 1) * columns + longitude + 1);  // upper right 
				mesh->indices.push_back((latitude + 1) * columns + longitude);  // upper left
				mesh->indices.push_back(latitude      * columns + longitude);  // lower left
			}
		}

		std::cout << "createSphere(): Vertices = " << mesh->attributes.size() << ", Triangles = " << mesh->indices.size() / 3 << std::endl;
		return mesh;
	}
	*/

	Mesh* Scene::createSphere(const int m_XSegments, const int m_YSegments, const float radius, const float maxTheta)
	{
		POptix::Mesh* mesh = new POptix::Mesh;

		mesh->attributes.reserve((m_YSegments + 1) * m_XSegments + 1);
		mesh->indices.reserve(6 * m_YSegments * (m_XSegments));

		for (uint32_t y = 0; y <= m_YSegments; ++y)
		{
			for (uint32_t x = 0; x <= m_XSegments; ++x)
			{
				float xSegment = (float)x / (float)m_YSegments;
				float ySegment = (float)y / (float)m_YSegments;
				auto xPos = static_cast<float>(radius * std::cos(xSegment * 2 * M_PIf) * std::sin(ySegment * M_PIf)); // TAU is 2PI
				auto yPos = static_cast<float>(radius * std::cos(ySegment * M_PIf));
				auto zPos = static_cast<float>(radius * std::sin(xSegment * 2 * M_PIf) * std::sin(ySegment * M_PIf));

				VertexAttributes attrib;
				attrib.vertex = make_float3(xPos, yPos, zPos);
				attrib.normal = make_float3(xPos, yPos, zPos);
				attrib.texcoord = make_float3(xSegment, ySegment, 0.0f);

				mesh->attributes.push_back(attrib);
			}
		}

		for (uint32_t y = 0; y < m_YSegments; ++y)
		{
			for (uint32_t x = 0; x < m_XSegments; ++x)
			{
				mesh->indices.push_back((y + 1) * (m_XSegments + 1) + x);
				mesh->indices.push_back(y       * (m_XSegments + 1) + x);
				mesh->indices.push_back(y       * (m_XSegments + 1) + x + 1);

				mesh->indices.push_back((y + 1) * (m_XSegments + 1) + x);
				mesh->indices.push_back(y       * (m_XSegments + 1) + x + 1);
				mesh->indices.push_back((y + 1) * (m_XSegments + 1) + x + 1);
			}
		}

		return mesh;
	}

}