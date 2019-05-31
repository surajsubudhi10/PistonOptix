#pragma once

#ifndef SCENE_H
#define SCENE_H

#include <vector>
#include <map>

#include "shaders\vertex_attributes.h"
#include "shaders\material_parameter.h"
#include "inc\LightParameters.h"
#include "inc\PinholeCamera.h"

using namespace std;

namespace POptix
{
	struct Properties
	{
		int width;
		int height;
		string sceneName;
	};

	struct Mesh
	{
		int ID;
		string filePath;
		string name;
		vector<VertexAttributes> attributes;
		vector<unsigned int> indices;
	};

	//using MeshPathPair = pair<Mesh*, std::string>;

	struct Node
	{
		std::string name;
		int materialID;
		vector<unsigned int> mMeshIDList;
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

		static Scene* LoadScene(const char* sceneFilePath);
		static Mesh* LoadOBJ(std::string inputfile);
	//private:
		Properties properties;
		vector<Node*> mNodeList;
		map<unsigned int, Mesh*> mMeshList;
		vector<Material*> mMaterialList;
		vector<Light*> mLightList;
		PinholeCamera* mCamera;
	};

	

}

#endif // SCENE_H
