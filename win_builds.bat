if not exist mkdir builds
cd builds

cmake -G "Visual Studio 15 2017 Win64" ..
cd ..

pause