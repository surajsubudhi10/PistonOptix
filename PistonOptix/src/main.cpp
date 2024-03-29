#include "shaders/app_config.h"
#include "inc/Application.h"
#include <sutil.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

static Application* g_app = nullptr;
static bool displayGUI = true;

static void error_callback(int error, const char* description)
{
	std::cerr << "Error: " << error << ": " << description << std::endl;
}

void printUsage(const std::string& argv0)
{
	std::cerr << "\nUsage: " << argv0 << " [options]\n";
	std::cerr <<
		"App Options:\n"
		"   ? | help | --help     Print this usage message and exit.\n"
		"  -w | --width <int>     Window client width  (512).\n"
		"  -h | --height <int>    Window client height (512).\n"
		"  -d | --devices <int>   OptiX device selection, each decimal digit selects one device (3210).\n"
		"  -n | --nopbo           Disable OpenGL interop for the image display.\n"
		"  -s | --stack <int>     Set the OptiX stack size (1024) (debug feature).\n"
		"  -f | --file <filename> Save image to file and exit.\n"
		"App Keystrokes:\n"
		"  SPACE  Toggles ImGui display.\n"
		"\n"
		<< std::endl;
}


int main(int argc, char *argv[])
{
	int  windowWidth = 1280;
	int  windowHeight = 720;
	int  devices = 3210;  // Decimal digits encode OptiX device ordinals. Default 3210 means to use all four first installed devices, when available.
	bool interop = true;  // Use OpenGL interop Pixel-Bufferobject to display the resulting image. Disable this when running on multi-GPU or TCC driver mode.
	int  stackSize = 1024;  // Command line parameter just to be able to find the smallest working size.
	std::string environment = std::string(sutil::samplesDir()) + "/data/NV_Default_HDR_3000x1500.hdr";

	std::string filenameScreenshot = "PistonOptix.png";
	bool showViewer = true;

	// Parse the command line parameters.
	for (int i = 1; i < argc; ++i)
	{
		const std::string arg(argv[i]);

		if (arg == "--help" || arg == "help" || arg == "?")
		{
			printUsage(argv[0]);
			return 0;
		}
		else if (arg == "-w" || arg == "--width")
		{
			if (i == argc - 1)
			{
				std::cerr << "Option '" << arg << "' requires additional argument.\n";
				printUsage(argv[0]);
				return 0;
			}
			windowWidth = atoi(argv[++i]);
		}
		else if (arg == "-h" || arg == "--height")
		{
			if (i == argc - 1)
			{
				std::cerr << "Option '" << arg << "' requires additional argument.\n";
				printUsage(argv[0]);
				return 0;
			}
			windowHeight = atoi(argv[++i]);
		}
		else if (arg == "-d" || arg == "--devices")
		{
			if (i == argc - 1)
			{
				std::cerr << "Option '" << arg << "' requires additional argument.\n";
				printUsage(argv[0]);
				return 0;
			}
			devices = atoi(argv[++i]);
		}
		else if (arg == "-s" || arg == "--stack")
		{
			if (i == argc - 1)
			{
				std::cerr << "Option '" << arg << "' requires additional argument.\n";
				printUsage(argv[0]);
				return 0;
			}
			stackSize = atoi(argv[++i]);
		}
		else if (arg == "-n" || arg == "--nopbo")
		{
			interop = false;
		}
		else if (arg == "-f" || arg == "--file")
		{
			if (i == argc - 1)
			{
				std::cerr << "Option '" << arg << "' requires additional argument.\n";
				printUsage(argv[0]);
				return 0;
			}
			filenameScreenshot = argv[++i];
			showViewer = false; // Do not render the GUI when just taking a screenshot. (Automated QA feature.)
		}
		else
		{
			std::cerr << "Unknown option '" << arg << "'\n";
			printUsage(argv[0]);
			return 0;
		}
	}

	glfwSetErrorCallback(error_callback);

	if (!glfwInit())
	{
		error_callback(1, "GLFW failed to initialize.");
		return 1;
	}

	glfwWindowHint(GLFW_VISIBLE, showViewer);
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "PistonOptix", nullptr, nullptr);
	if (!window)
	{
		error_callback(2, "glfwCreateWindow() failed.");
		glfwTerminate();
		return 2;
	}

	glfwMakeContextCurrent(window);

	if (glewInit() != GL_NO_ERROR)
	{
		error_callback(3, "GLEW failed to initialize.");
		glfwTerminate();
		return 3;
	}

	g_app = new Application(window, windowWidth, windowHeight,
		devices, stackSize, interop);

	if (!g_app->isValid())
	{
		error_callback(4, "Application initialization failed.");
		glfwTerminate();
		return 4;
	}

	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents(); // Render continuously.

		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

		g_app->reshape(windowWidth, windowHeight);

		if (showViewer)
		{
			g_app->guiNewFrame();

			//g_app->guiReferenceManual(); // DAR HACK The ImGui "Programming Manual" as example code.

			g_app->guiWindow(); // The OptiX introduction example GUI window.

			g_app->guiEventHandler(); // Currently only reacting on SPACE to toggle the GUI window.

			g_app->render();  // OptiX rendering and OpenGL texture update.
			g_app->display(); // OpenGL display.

			g_app->guiRender(); // Render all ImGUI elements at last.

			glfwSwapBuffers(window);
		}
		else
		{
			for (int i = 0; i < 100; ++i) // Accumulate 64 samples per pixel.
			{
				g_app->render();  // OptiX rendering and OpenGL texture update.
			}
			g_app->screenshot(filenameScreenshot);

			glfwSetWindowShouldClose(window, 1);
		}

		//glfwWaitEvents(); // Render only when an event is happening. Needs some glfwPostEmptyEvent() to prevent GUI lagging one frame behind when ending an action.
	}

	// Cleanup
	delete g_app;

	glfwTerminate();

	return 0;
}

