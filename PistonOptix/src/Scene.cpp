#include "inc/Scene.h"

namespace POptix
{
	Scene::Scene()
	{
		build();
	}
	Scene::~Scene()
	{
		for each (Node* node in mNodeList)
		{
			for each (Mesh* mesh in node->mMeshList)
			{
				delete mesh;
			}
			delete node;
		}

		for each (Light* light in mLightList)
		{
			delete light;
		}

		for each (Material* mat in mMaterialList)
		{
			delete mat;
		}

		delete mCamera;
	}
	
	void Scene::build()
	{
		/////////////////////////////////////
		//		Creating Plane             //
		/////////////////////////////////////

		// Make mesh
		Mesh* planeMesh = Scene::createPlane(1, 1, 1);
		float tranOfPlane[16] =
		{
		  10.0f, 0.0f, 0.0f, /*tx*/0.0f,
		  0.0f, 10.0f, 0.0f, /*ty*/0.0f,
		  0.0f, 0.0f, 10.0f, /*tz*/0.0f,
		  0.0f, 0.0f, 0.0f, 1.0f
		};

		Node* planeNode = new Node();
		planeNode->mMeshList.emplace_back(planeMesh);
		planeNode->name = "Sphere Mesh";
		planeNode->materialID = 2;
		for (int i = 0; i < 16; i++)
			planeNode->transform[i] = tranOfPlane[i];
		mNodeList.emplace_back(planeNode);

		/////////////////////////////////////
		//		Creating Sphere            //
		/////////////////////////////////////

		// Make mesh
		const int tessU = 50;
		const int tessV = 50;
		const float radius = 2.0f;
		const float maxTheta = M_PIf;
		Mesh* sphereMesh = Scene::createSphere(tessU, tessV, radius, maxTheta);

		float tranOfSphere[16] =
		{
		  1.0f, 0.0f, 0.0f, /*tx*/0.0f,
		  0.0f, 1.0f, 0.0f, /*ty*/2.0f,
		  0.0f, 0.0f, 1.0f, /*tz*/-3.0f,
		  0.0f, 0.0f, 0.0f, 1.0f
		};

		Node* sphereNode = new Node();
		sphereNode->mMeshList.emplace_back(sphereMesh);
		sphereNode->name = "Sphere Mesh";
		sphereNode->materialID = 0;
		for(int i = 0; i < 16; i++)
			sphereNode->transform[i] = tranOfSphere[i];
		mNodeList.emplace_back(sphereNode);

		/////////////////////////////////////
		//		Creating Torus             //
		/////////////////////////////////////

		const float innerRadius = 3.0f;
		const float outerRadius = 1.0f;
		Mesh* torusMesh = Scene::createTorus(tessU, tessV, innerRadius, outerRadius);
		
		float tranOfTorus[16] =
		{
		  1.0f, 0.0f, 0.0f, /*tx*/0.0f,
		  0.0f, 1.0f, 0.0f, /*ty*/3.0f,
		  0.0f, 0.0f, 1.0f, /*tz*/6.0f,
		  0.0f, 0.0f, 0.0f, 1.0f
		};

		Node* torusNode = new Node();
		torusNode->mMeshList.emplace_back(torusMesh);
		torusNode->name = "Torus Mesh";
		torusNode->materialID = 1;
		for (int i = 0; i < 16; i++)
			torusNode->transform[i] = tranOfTorus[i];
		mNodeList.emplace_back(torusNode);

		/////////////////////////////////////
		//		Creating Materials         //
		/////////////////////////////////////

		Material* mat01 = new Material();
		mat01->albedo = make_float3(1.0f, 0.0f, 0.0f);
		mat01->metallic = 1.0f;
		mat01->roughness = 0.13f;
		mMaterialList.emplace_back(mat01);

		Material* mat02 = new Material();
		mat02->albedo = make_float3(0.0f, 1.0f, 1.0f);
		mat02->metallic = 0.0f;
		mat02->roughness = 0.0f;
		mMaterialList.emplace_back(mat02);

		Material* mat03 = new Material();
		mat03->albedo = make_float3(0.0f, 0.0f, 1.0f);
		mat03->metallic = 1.0f;
		mat03->roughness = 1.0f;
		mMaterialList.emplace_back(mat03);

		/////////////////////////////////////
		//		Creating Lights            //
		/////////////////////////////////////
		Light* directionalLight = new Light();
		directionalLight->emission = optix::make_float3(10.0f, 10.0f, 10.0f);
		directionalLight->lightType = POptix::ELightType::DIRECTIONAL;
		directionalLight->direction = optix::normalize(optix::make_float3(-1.0f, 1.0f, 1.0f));
		mLightList.emplace_back(directionalLight);

		mCamera = new PinholeCamera();
	}

	void Scene::LoadScene(std::string sceneFilePath)
	{
	}
}
