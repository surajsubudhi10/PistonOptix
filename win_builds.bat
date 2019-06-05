@echo off
SET OptiX_Path=%1

if not defined OptiX_Path (SET OptiX_Path="C:\\ProgramData\\NVIDIA Corporation\\OptiX SDK 5.1.1")

@echo OPTIX_PATH : %OptiX_Path%

if not exist builds (mkdir builds)
cd builds

cmake -G "Visual Studio 15 2017 Win64" -DOptiX_INSTALL_DIR=%Optix_Path% ..
cd ..

pause