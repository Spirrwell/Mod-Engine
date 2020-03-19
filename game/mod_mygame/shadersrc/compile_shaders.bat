SET glslValidator=%VULKAN_SDK%\Bin\glslangValidator.exe

%glslValidator% -V BasicShader.vert -o ../shaders/BasicShader.vert.spv
%glslValidator% -V BasicShader.frag -o ../shaders/BasicShader.frag.spv

%glslValidator% -V WireShader.vert -o ../shaders/WireShader.vert.spv
%glslValidator% -V WireShader.frag -o ../shaders/WireShader.frag.spv