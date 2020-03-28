SET glslValidator=%VULKAN_SDK%\Bin\glslangValidator.exe

%glslValidator% -V StaticMesh.vert -o ../shaders/StaticMesh.vert.spv
%glslValidator% -V StaticMesh.frag -o ../shaders/StaticMesh.frag.spv

%glslValidator% -V Wireframe.vert -o ../shaders/Wireframe.vert.spv
%glslValidator% -V Wireframe.frag -o ../shaders/Wireframe.frag.spv