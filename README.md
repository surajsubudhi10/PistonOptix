## PistonOptix

A Real time Path Tracer using Monte Carlo simulation for rendering based on NVIDIA Optix SDK and using GLFW and OpenGL for viewer. The feature set is very basic since this is just a learning exercise and inspired from Nvidia's Optix Advanced Samples.

## Quick Start
* You will need CMake to build the code. As of now this project only support Windows, so you need Visual Studio 2015 or 2017 in addition to CMake. 
* Install Optix 5.0 or higher and CUDA v9.2 or higher.

First, clone the code:
```
git clone https://github.com/surajsubudhi10/PistonOptix.git --recursive
cd PistonOptix
```

Now into the project folder run the `win_builds.bat` file in command line.
If you have installed Optix SDK in different directory or different version (default Optix 5.1.1) then you need to add the path of the Optix direction along with `win_builds.bat` batch file. 

```
\PistonOptix>.\win_builds.bat "< Optix SDK path inside double quote >"
```
It will generate .sln files along with separate visual studio projects file. Run the `PistonOptix.sln` file.


## Features

* Unidirectional Path Tracing
* Lambertian and Microfacet BRDF
* Rect and Directional lights
* Multiple Importance Sampling
* Mesh Loading
* Loading scene from .scn file

## Examples
Here are some of the rendered snaps form the project.

![Bunny](./resources/Samples/Sample01.png)