# Mod-Engine

Mod-Engine is a work in progress game engine designed with moddability in mind. This engine is currently not in a usable state.

# Dependencies

### Windows:
Install the latest Vulkan SDK for Windows from here: https://vulkan.lunarg.com/sdk/home#windows and you're good to go! All other dependencies are included.

Visual Studio 2019 is supported, Visual Studio 2017 may work but is not supported.

### Linux:
Consult your distros package manager and install the following packages

* SDL2
* fmtlib
* assimp

You will also want to download the latest Vulkan SDK from here: https://vulkan.lunarg.com/sdk/home#linux The README inside the Vulkan tarball contains a link to the guide for that version. Most importantly, follow the instructions for setting up the runtime environment. This will define the VULKAN_SDK environment variable so Mod-Engine's premake script can find Vulkan.

You will want at least GCC/G++ 7 or at least clang 5 as this project relies on C++17 features.

# Building
This project uses premake5 for its build system. You will want to download the latest version of premake5 for your OS from here: https://premake.github.io/download.html

Generating projects should be as simple as doing premake5 vs2019 or premake5 gmake
