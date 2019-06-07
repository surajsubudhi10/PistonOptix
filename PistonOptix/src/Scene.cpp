#include "inc/Scene.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <iostream>
#include <sutil.h>

namespace POptix
{
	static std::string getFileName(const std::string& filePath)
	{
		char sep = '/';

#ifdef _WIN32
		sep = '\\';
#endif

		size_t i = filePath.rfind(sep, filePath.length());
		if (i != std::string::npos)
		{
			return(filePath.substr(i + 1, filePath.length() - i));
		}

		return("");
	}

	static std::string getDirectoryPath(const std::string& filename)
	{
		char sep = '/';

#ifdef _WIN32
		sep = '\\';
#endif

		const size_t last_slash_idx = filename.rfind(sep);
		if (std::string::npos != last_slash_idx)
		{
			return filename.substr(0, last_slash_idx);
		}

		return("");
	}

	static std::string getFileExtension(const std::string& filename)
	{
		char sep = '.';

		const size_t last_dot_idx = filename.rfind(sep);
		if (last_dot_idx != std::string::npos)
		{
			return(filename.substr(last_dot_idx + 1, filename.length() - last_dot_idx));
		}

		return("");
	}

	Scene::Scene()
	{
		//build();
	}
	Scene::~Scene()
	{
		for each (Node* node in mNodeList)
		{
			delete node;
		}

		std::map<unsigned int, POptix::Mesh*>::iterator it;
		for (it = mMeshList.begin(); it != mMeshList.end(); ++it) 
		{
			delete it->second;
		}
		mMeshList.clear();

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
		mMeshList.insert(make_pair(0, planeMesh));
		float tranOfPlane[16] =
		{
		  10.0f, 0.0f, 0.0f, /*tx*/0.0f,
		  0.0f, 10.0f, 0.0f, /*ty*/0.0f,
		  0.0f, 0.0f, 10.0f, /*tz*/0.0f,
		  0.0f, 0.0f, 0.0f, 1.0f
		};

		Node* planeNode = new Node();
		planeNode->mMeshIDList.emplace_back(0);
		planeNode->name = "Plane Mesh";
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
		mMeshList.insert(make_pair(1, sphereMesh));
		float tranOfSphere[16] =
		{
		  1.0f, 0.0f, 0.0f, /*tx*/0.0f,
		  0.0f, 1.0f, 0.0f, /*ty*/2.0f,
		  0.0f, 0.0f, 1.0f, /*tz*/-3.0f,
		  0.0f, 0.0f, 0.0f, 1.0f
		};

		Node* sphereNode = new Node();
		sphereNode->mMeshIDList.emplace_back(1);
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
		mMeshList.insert(make_pair(2, torusMesh));
		float tranOfTorus[16] =
		{
		  1.0f, 0.0f, 0.0f, /*tx*/0.0f,
		  0.0f, 1.0f, 0.0f, /*ty*/3.0f,
		  0.0f, 0.0f, 1.0f, /*tz*/6.0f,
		  0.0f, 0.0f, 0.0f, 1.0f
		};

		Node* torusNode = new Node();
		torusNode->mMeshIDList.emplace_back(2);
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

	static const int kMaxLineLength = 2048;
	Scene* Scene::LoadScene(const char* sceneFilePath)
	{
		auto fileexten = getFileExtension(sceneFilePath);
		if(getFileExtension(sceneFilePath) != "scn")
		{
			std::cerr << "Error! Only supports .scn file. \n";
			exit(1);
		}

		FILE* file = fopen(sceneFilePath, "r");
		if (!file)
		{
			printf("Couldn't open %s for reading.", sceneFilePath);
			return nullptr;
		}

		Scene *scene = new Scene;
		Properties prop;
		prop.width = 1280;
		prop.height = 720;
		prop.sceneName = getFileName(sceneFilePath);
		prop.sceneDirectoryPath = getDirectoryPath(sceneFilePath);
		scene->properties = prop;

		char line[kMaxLineLength];
		while (fgets(line, kMaxLineLength, file)) 
		{
			// skip comments
			if (line[0] == '#')
				continue;

			//------------------------------------------//
			//				Properties					//
			//------------------------------------------//
			if (strstr(line, "properties"))
			{

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " width %i", &prop.width);
					sscanf(line, " height %i", &prop.height);
					sscanf(line, " name %s", &prop.sceneName);
				}
				scene->properties = prop;
			}

			// name used for materials and meshes
			char name[kMaxLineLength] = { 0 };

			//------------------------------------------//
			//				Material					//
			//------------------------------------------//
			if (sscanf(line, " material %s", name) == 1)
			{
				printf("%s", line);

				Material* material = new Material;
				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " name %s", name);
					sscanf(line, " color %f %f %f", &material->albedo.x, &material->albedo.y, &material->albedo.z);
					sscanf(line, " metallic %f",  &material->metallic);
					sscanf(line, " roughness %f", &material->roughness);
				}

				scene->mMaterialList.emplace_back(material);
			}

			//------------------------------------------//
			//				Light						//
			//------------------------------------------//
			if (strstr(line, "light"))
			{
				Light* light = new Light;
				light->position = make_float3(0.0f);
				light->radius = 1.0f;
				light->u = make_float3(1.0f, 0.0f, 0.0f);
				light->v = make_float3(0.0f, 0.0f, 1.0f);

				optix::float3 v1, v2;
				char light_type[20] = "None";

				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " position %f %f %f", &light->position.x, &light->position.y, &light->position.z);
					sscanf(line, " emission %f %f %f", &light->emission.x, &light->emission.y, &light->emission.z);
					sscanf(line, " direction %f %f %f", &light->direction.x, &light->direction.y, &light->direction.z);

					sscanf(line, " radius %f", &light->radius);
					sscanf(line, " u %f %f %f", &light->u.x, &light->u.y, &light->u.z);
					sscanf(line, " v %f %f %f", &light->v.x, &light->v.y, &light->v.z);
					sscanf(line, " type %s", light_type);
				}

				if (strcmp(light_type, "Quad") == 0)
				{
					light->lightType = QUAD;
					light->u = v1 - light->position;
					light->v = v2 - light->position;
					light->area = optix::length(optix::cross(light->u, light->v));
					light->direction = optix::normalize(optix::cross(light->u, light->v));
				}
				else if (strcmp(light_type, "Sphere") == 0)
				{
					light->lightType = SPHERE;
					light->direction = optix::normalize(light->direction);
					light->area = 4.0f * M_PIf * light->radius * light->radius;
				}
				else if (strcmp(light_type, "Directional") == 0)
				{
					light->lightType = DIRECTIONAL;
					light->direction = optix::normalize(light->direction);
				}
				else if (strcmp(light_type, "Environment") == 0)
				{
					//light.lightType = ENVIRONMENT;
				}

				scene->mLightList.emplace_back(light);
			}

			//------------------------------------------//
			//				Mesh						//
			//------------------------------------------//
			if (sscanf(line, " mesh %s", name) == 1)
			{
				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " name %s", name);
					int count = 0;
					char path[2048];

					if (sscanf(line, " filepath %s", path) == 1)
					{
						unsigned int meshCount = scene->mMeshList.size();
						
						string OBJfilePath = scene->properties.sceneDirectoryPath + "\\" + path;
						Mesh* mesh = LoadOBJ(OBJfilePath);
						mesh->name = name;
						mesh->filePath = path;
						mesh->ID = meshCount;
						scene->mMeshList.insert(make_pair(meshCount, mesh));
					}
				}
			}

			//------------------------------------------//
			//				Node						//
			//------------------------------------------//
			if (sscanf(line, " node %s", name) == 1)
			{
				printf("%s", line);

				Node* node = new Node;
				node->transform = new float[16];
				node->name = name;
				while (fgets(line, kMaxLineLength, file))
				{
					// end group
					if (strchr(line, '}'))
						break;

					sscanf(line, " materialID %d", &node->materialID);
					sscanf(line, " transform %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f", 
						&node->transform[ 0], &node->transform[ 1], &node->transform[ 2], &node->transform[ 3], 
						&node->transform[ 4], &node->transform[ 5], &node->transform[ 6], &node->transform[ 7], 
						&node->transform[ 8], &node->transform[ 9], &node->transform[10], &node->transform[11], 
						&node->transform[12], &node->transform[13], &node->transform[14], &node->transform[15]);

					
					unsigned int val = 0;
					if (sscanf(line, " meshID %d", &val))
					{
						// https://www.geeksforgeeks.org/extract-integers-string-c/
						stringstream ss;
						string meshIDString = std::string(line);

						/* Storing the whole string into string stream */
						ss << meshIDString;

						/* Running loop till the end of the stream */
						string temp;
						int found;
						while (!ss.eof()) {

							/* extracting word by word from stream */
							ss >> temp;

							/* Checking the given word is integer or not */
							if (stringstream(temp) >> found) 
							{
								cout << found << " ";
								node->mMeshIDList.push_back(found);
							}

							/* To save from space at the end of string */
							temp = "";
						}
					}
				}
				scene->mNodeList.emplace_back(node);
			}
		}
		return scene;
	}


	Mesh* Scene::LoadOBJ(std::string inputfile)
	{
		Mesh* mesh = new Mesh;

		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;

		std::string err;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, inputfile.c_str());

		if (!err.empty() && err[0] != 'W') { // `err` may contain warning message.
			std::cerr << err << std::endl;
		}

		if (!ret)
			exit(1);


		// Loop over shapes
		size_t numOfIndicesInShape = 0;
		for (auto& shape : shapes)
		{
			// Loop over faces(polygon)
			size_t index_offset = 0;
			for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
			{
				int fv = shape.mesh.num_face_vertices[f];

				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++)
				{
					// access to vertex
					tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
					VertexAttributes singleVertexData;

					tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
					tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
					tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];
					tinyobj::real_t nx = attrib.normals[3 * idx.normal_index + 0];
					tinyobj::real_t ny = attrib.normals[3 * idx.normal_index + 1];
					tinyobj::real_t nz = attrib.normals[3 * idx.normal_index + 2];

					tinyobj::real_t tx = 0.0f;
					tinyobj::real_t ty = 1.0f;
					if (idx.texcoord_index != -1)
					{
						tx = attrib.texcoords[2 * idx.texcoord_index + 0];
						ty = attrib.texcoords[2 * idx.texcoord_index + 1];
					}

					singleVertexData.vertex = optix::make_float3(vx, vy, vz);
					singleVertexData.normal = optix::make_float3(nx, ny, nz);
					singleVertexData.texcoord = optix::make_float3(tx, ty, 0.0f);

					mesh->attributes.push_back(singleVertexData);
					mesh->indices.push_back((unsigned int)(numOfIndicesInShape + index_offset + v));
				}
				index_offset += fv;

				// per-face material
				shape.mesh.material_ids[f];
			}
			numOfIndicesInShape += index_offset;
		}

		std::cout << "LoadOBJ(" << getFileName(inputfile) << "): Vertices = " << mesh->attributes.size() << ", Triangles = " << mesh->indices.size() / 3 << std::endl;
		return mesh;
	}
}
