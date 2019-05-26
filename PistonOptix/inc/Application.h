#pragma once

#ifndef APPLICATION_H
#define APPLICATION_H

#if defined(_WIN32)
#include <windows.h>
#endif

#include <imgui/imgui.h>

#define IMGUI_DEFINE_MATH_OPERATORS 1
#include <imgui/imgui_internal.h>

#include <imgui/imgui_impl_glfw_gl2.h>

#ifndef __APPLE__
#  include <GL/glew.h>
#  if defined( _WIN32 )
#    include <GL/wglew.h>
#  endif
#endif

#include <GLFW/glfw3.h>

#include <optix.h>
#include <optixu/optixpp_namespace.h>

#include "inc/PinholeCamera.h"
#include "inc/Timer.h"

#include "shaders/vertex_attributes.h"
#include "shaders/material_parameter.h"

#include <string>
#include <map>


using namespace optix;

 // For rtDevice*() function error checking. No OptiX context present at that time.
#define RT_CHECK_ERROR_NO_CONTEXT( func ) \
  do { \
    RTresult code = func; \
    if (code != RT_SUCCESS) \
      std::cerr << "ERROR: Function " << #func << std::endl; \
  } while (0)


enum GuiState
{
	GUI_STATE_NONE,
	GUI_STATE_ORBIT,
	GUI_STATE_PAN,
	GUI_STATE_DOLLY,
	GUI_STATE_FOCUS
};

// Host side GUI material parameters.
// (The separation between host side GUI parameters and the MaterialParameters struct on device side 
//  will become clearer in later examples, where GUI parameters are not just copied´to the buffer.)
struct MaterialParameterGUI
{
	optix::float3 albedo; // albedo, color, tint, throughput change for specular materials.
	float metallic;
	float roughness;
};


class Application
{
public:
	Application(GLFWwindow* window,
		const int width,
		const int height,
		const unsigned int devices,
		const unsigned int stackSize,
		const bool interop);
	~Application();

	bool isValid() const;

	void reshape(int width, int height);

	bool render();
	void display() const;

	void screenshot(std::string const& filename);

	void guiNewFrame();
	void guiWindow();
	void guiEventHandler();
	void guiRender();

	static void guiReferenceManual(); // DAR HACK DEBUG The IMGUI "programming manual" in form of a live window.

private:
	static void getSystemInformation();

	void initOpenGL();
	void checkInfoLog(const char *msg, GLuint object);
	void initGLSL();

	void initOptiX();
	void initRenderer();
	void initPrograms();
	void initMaterials();
	void initScene();

	void createScene();

	optix::Geometry createBox();
	optix::Geometry createPlane(const int tessU, const int tessV, const int upAxis);
	optix::Geometry createSphere(const int tessU, const int tessV, const float radius, const float maxTheta);
	optix::Geometry createTorus(const int tessU, const int tessV, const float innerRadius, const float outerRadius);
	optix::Geometry LoadOBJ(std::string objPath);

	void createGeometryFromOBJ(std::string objPath, uint materialID, float* transform);
	optix::Geometry createGeometry(std::vector<VertexAttributes> const& attributes, std::vector<unsigned int> const& indices);

	void setAccelerationProperties(optix::Acceleration acceleration);

	void updateMaterialParameters();

	void restartAccumulation();

private:
	GLFWwindow* m_window;

	int         m_width;
	int         m_height;

	bool        m_isValid;

	// Application command line parameters.
	unsigned int m_devicesEncoding;
	unsigned int m_stackSize;
	bool         m_interop;

	// Application GUI parameters.
	int   m_minPathLength;       // Minimum path length after which Russian Roulette path termination starts.
	int   m_maxPathLength;       // Maximum path length.
	float m_sceneEpsilonFactor;  // Factor on 1e-7 used to offset ray origins along the path to reduce self intersections. 

	int   m_iterationIndex;

	std::string m_builder;

	// OpenGL variables:
	GLuint m_pboOutputBuffer;
	GLuint m_hdrTexture;

	// OptiX variables:
	optix::Context m_context;

	// Output Buffer
	optix::Buffer m_bufferOutput;

	// BRDF Buffers
	optix::Buffer m_bufferBRDFSample;
	optix::Buffer m_bufferBRDFEval;
	optix::Buffer m_bufferBRDFPdf;

	std::map<std::string, optix::Program> m_mapOfPrograms;

	// The material parameters exposed inside the GUI are slightly different than the resulting values for the device.
	// The GUI exposes an absorption color and a distance scale, and the thin-walled property as bool.
	// These are converted on the fly into the device side sysMaterialParameters buffer.
	std::vector<MaterialParameterGUI> m_guiMaterialParameters;
	optix::Buffer                     m_bufferMaterialParameters; // Array of MaterialParameters.

	bool   m_present; // This controls if the texture image is updated per launch or only once a second.
	bool   m_presentNext;
	double m_presentAtSecond;

	int m_frames;

	// GLSL shaders objects and program.
	GLuint m_glslVS;
	GLuint m_glslFS;
	GLuint m_glslProgram;

	// Tonemapper group:
	float         m_gamma;
	optix::float3 m_colorBalance;
	float         m_whitePoint;
	float         m_burnHighlights;
	float         m_crushBlacks;
	float         m_saturation;
	float         m_brightness;
	bool		  m_useTonermapper;

	GuiState m_guiState;

	bool m_isWindowVisible; // Hide the GUI window completely with SPACE key.

	float m_mouseSpeedRatio;

	PinholeCamera m_pinholeCamera;

	Timer m_timer;

	optix::Material m_opaqueMaterial;

	// The root node of the OptiX scene graph (sysTopObject)
	optix::Group        m_rootGroup;
	optix::Acceleration m_rootAcceleration;
};

#endif // APPLICATION_H

