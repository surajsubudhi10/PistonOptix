#pragma once

#ifndef SCENE_H
#define SCENE_H

#include <vector>

#include "shaders\vertex_attributes.h"
#include "shaders\material_parameter.h"
#include "inc\LightParameters.h"
#include "inc\PinholeCamera.h"

using namespace std;

namespace POptix
{
	struct Mesh
	{
		vector<VertexAttributes> attributes;
		vector<unsigned int> indices;
	};

	struct Node
	{
		std::string name;
		vector<Mesh*> mMeshList;
		int materialID;
		float* transform = new float[16];
	};

	class Scene
	{
	public:
		Scene();
		~Scene();

		void build();

		// for debuging
		static POptix::Mesh* createBox();
		static POptix::Mesh* createPlane(const int tessU, const int tessV, const int upAxis);
		static POptix::Mesh* createSphere(const int tessU, const int tessV, const float radius, const float maxTheta);
		static POptix::Mesh* createTorus(const int tessU, const int tessV, const float innerRadius, const float outerRadius);

		static void LoadScene(std::string sceneFilePath);

	//private:
		vector<Node*> mNodeList;
		vector<Material*> mMaterialList;
		vector<Light*> mLightList;
		PinholeCamera* mCamera;
	};

	

}

#endif // SCENE_H
