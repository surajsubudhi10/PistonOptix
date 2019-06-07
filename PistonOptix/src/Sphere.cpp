#include "inc/Application.h"

#include <cstring>
#include <iostream>
#include <sstream>

#include "inc/MyAssert.h"
#include "inc/Scene.h"

namespace POptix
{
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