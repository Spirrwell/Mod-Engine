function path_error( env_name )
	premake.error( env_name .. " path not found." )
end

VULKAN_SDK = os.getenv( "VULKAN_SDK" )

if VULKAN_SDK == nil then
	path_error( "VULKAN_SDK" )
end

VULKAN_INCLUDE_DIR = VULKAN_SDK .. "/include"
VULKAN_LIBRARY_DIR = VULKAN_SDK .. "/lib"
VULKAN_LIBRARY = "vulkan"
VC_VULKAN_LIBRARY = "vulkan-1"

VC_BIN_DIR = "./dependencies/x64-windows/bin"
VC_DEBUG_BIN_DIR = "./dependencies/x64-windows/debug/bin"

VC_INCLUDE_DIR = "./dependencies/x64-windows/include"
--VC_DEBUG_INCLUDE_DIR = "./dependencies/x64-windows/debug/include"

VC_LIB_DIR = "./dependencies/x64-windows/lib"
VC_DEBUG_LIB_DIR = "./dependencies/x64-windows/debug/lib"

DEPENDENCY_DIR = path.getabsolute( "./dependencies" )
GAME_DIR = path.getabsolute( "./game" )
GAME_BIN_DIR = path.getabsolute( "./game/mod_mygame/bin" )

INTERFACES_SOURCE_DIR = "./src/interfaces"

SHARED_SOURCE_DIR = "./src/shared"
ENGINE_SOURCE_DIR = "./src/engine"
GAME_SOURCE_DIR = "./src/game"

INTERFACES_SOURCE_FILES = {
	INTERFACES_SOURCE_DIR .. "/engine/iengine.hpp",
	INTERFACES_SOURCE_DIR .. "/game/igame.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/ilogengine.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/imaterial.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/imaterialsystem.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/imesh.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/imodel.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/imodelsystem.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/irenderer.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/iresourcepool.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/ishader.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/ishadersystem.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/itexture.hpp",
	INTERFACES_SOURCE_DIR .. "/engine/itexturesystem.hpp"
}

SHARED_SOURCE_FILES = {
	SHARED_SOURCE_DIR .. "/color.cpp",
	SHARED_SOURCE_DIR .. "/color.hpp",
	SHARED_SOURCE_DIR .. "/log.cpp",
	SHARED_SOURCE_DIR .. "/log.hpp",
	SHARED_SOURCE_DIR .. "/memory.hpp",
	SHARED_SOURCE_DIR .. "/renderview.hpp",
}

workspace "ModEngine"
	architecture "x64"
	configurations { "Debug", "Release" }
	startproject "engine"
	location "build"
	
	vpaths {
		{ [ "Interfaces/*" ] = { INTERFACES_SOURCE_DIR .. "/**.hpp", INTERFACES_SOURCE_DIR .. "**.cpp" } },
		{ [ "Shared/Source Files/*" ] = { SHARED_SOURCE_DIR .. "**.cpp" } },
		{ [ "Shared/Header Files/*" ] = { SHARED_SOURCE_DIR .. "**.hpp" } },
		{ [ "Header Files" ] = { "**.h", "**.hpp" } },
		{ [ "Source Files" ] = { "**.cpp" } }
	}
	
	includedirs {
		"./dependencies/any-any/include",
		SHARED_SOURCE_DIR,
		INTERFACES_SOURCE_DIR
	}
	
	defines { "_CRT_SECURE_NO_WARNINGS", "NOMINMAX", "GLM_FORCE_DEFAULT_ALIGNED_GENTYPES", "GLM_FORCE_DEPTH_ZERO_TO_ONE" }
	
	filter { "configurations:Debug" }
		symbols "On"
		optimize "Off"
	
	filter { "configurations:Release" }
		symbols "Off"
		optimize "On"
	
	filter { "action:vs*", "configurations:Debug" }
		libdirs { VC_DEBUG_LIB_DIR }
	
	filter { "action:vs*" }
		includedirs { VC_INCLUDE_DIR }
		libdirs { VC_LIB_DIR }
		
project "game"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	targetname "game"
    targetprefix ""
	location "build/game"
	
	files {
		SHARED_SOURCE_FILES,
		INTERFACES_SOURCE_FILES
	}
	
	files {
		GAME_SOURCE_DIR .. "/camera.hpp",
		GAME_SOURCE_DIR .. "/ecs.cpp",
		GAME_SOURCE_DIR .. "/game.cpp",
		GAME_SOURCE_DIR .. "/game.hpp",
		GAME_SOURCE_DIR .. "/module_main.cpp"
	}
	
	postbuildcommands {
		"{COPY} %{cfg.buildtarget.abspath} %{GAME_BIN_DIR}/%{cfg.buildtarget.name}"
	}
	
	filter { "configurations:Release", "action:vs*" }
		links {
			"fmt"
		}
	
	filter { "configurations:Debug", "action:vs*" }
		links {
			"fmtd"
		}
	
	filter { "action:not vs*" }
		links {
			"stdc++fs",
			"fmt"
		}
	
	filter { "toolset:gcc", "system:windows" }
		links {
			"mingw32"
		}

project "engine"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetname "engine"
	location "build/engine"
	
	files {
		SHARED_SOURCE_FILES,
		INTERFACES_SOURCE_FILES
	}
	
	files {
		ENGINE_SOURCE_DIR .. "/clock.hpp",
		ENGINE_SOURCE_DIR .. "/commandlinesystem.cpp",
		ENGINE_SOURCE_DIR .. "/commandlinesystem.hpp",
		ENGINE_SOURCE_DIR .. "/engine.cpp",
		ENGINE_SOURCE_DIR .. "/engine.hpp",
		ENGINE_SOURCE_DIR .. "/enginesystem.cpp",
		ENGINE_SOURCE_DIR .. "/enginesystem.hpp",
		ENGINE_SOURCE_DIR .. "/vfile.cpp",
		ENGINE_SOURCE_DIR .. "/vfile.hpp",
		ENGINE_SOURCE_DIR .. "/filesystem.cpp",
		ENGINE_SOURCE_DIR .. "/filesystem.hpp",
		ENGINE_SOURCE_DIR .. "/inputevent.hpp",
		ENGINE_SOURCE_DIR .. "/inputsystem.cpp",
		ENGINE_SOURCE_DIR .. "/inputsystem.hpp",
		ENGINE_SOURCE_DIR .. "/main.cpp",
		ENGINE_SOURCE_DIR .. "/material.cpp",
		ENGINE_SOURCE_DIR .. "/material.hpp",
		ENGINE_SOURCE_DIR .. "/materialsystem.cpp",
		ENGINE_SOURCE_DIR .. "/materialsystem.hpp",
		ENGINE_SOURCE_DIR .. "/mesh.cpp",
		ENGINE_SOURCE_DIR .. "/mesh.hpp",
		ENGINE_SOURCE_DIR .. "/model.cpp",
		ENGINE_SOURCE_DIR .. "/model.hpp",
		ENGINE_SOURCE_DIR .. "/modelsystem.cpp",
		ENGINE_SOURCE_DIR .. "/modelsystem.hpp",
		ENGINE_SOURCE_DIR .. "/modulesystem.cpp",
		ENGINE_SOURCE_DIR .. "/modulesystem.hpp",
		ENGINE_SOURCE_DIR .. "/mount.hpp",
		ENGINE_SOURCE_DIR .. "/vulkansystem.cpp",
		ENGINE_SOURCE_DIR .. "/vulkansystem.hpp",
		ENGINE_SOURCE_DIR .. "/renderview.hpp",
		ENGINE_SOURCE_DIR .. "/resource.hpp",
		ENGINE_SOURCE_DIR .. "/resourcepool.cpp",
		ENGINE_SOURCE_DIR .. "/resourcepool.hpp",
		ENGINE_SOURCE_DIR .. "/renderer.cpp",
		ENGINE_SOURCE_DIR .. "/renderer.hpp",
		ENGINE_SOURCE_DIR .. "/sdlwindow.cpp",
		ENGINE_SOURCE_DIR .. "/sdlwindow.hpp",
		ENGINE_SOURCE_DIR .. "/sdlwrapper.cpp",
		ENGINE_SOURCE_DIR .. "/sdlwrapper.hpp",
		ENGINE_SOURCE_DIR .. "/shader_staticmesh.cpp",
		ENGINE_SOURCE_DIR .. "/shader_staticmesh.hpp",
		ENGINE_SOURCE_DIR .. "/shader_wireframe.cpp",
		ENGINE_SOURCE_DIR .. "/shader_wireframe.hpp",
		ENGINE_SOURCE_DIR .. "/shader.cpp",
		ENGINE_SOURCE_DIR .. "/shader.hpp",
		ENGINE_SOURCE_DIR .. "/shadersystem.cpp",
		ENGINE_SOURCE_DIR .. "/shadersystem.hpp",
		ENGINE_SOURCE_DIR .. "/state.hpp",
		ENGINE_SOURCE_DIR .. "/stb_filesystem.cpp",
		ENGINE_SOURCE_DIR .. "/stb_filesystem.hpp",
		ENGINE_SOURCE_DIR .. "/stb_implementation.cpp",
		ENGINE_SOURCE_DIR .. "/texture.cpp",
		ENGINE_SOURCE_DIR .. "/texture.hpp",
		ENGINE_SOURCE_DIR .. "/texturesystem.cpp",
		ENGINE_SOURCE_DIR .. "/texturesystem.hpp",
		ENGINE_SOURCE_DIR .. "/thread.cpp",
		ENGINE_SOURCE_DIR .. "/thread.hpp",
		ENGINE_SOURCE_DIR .. "/ubo.cpp",
		ENGINE_SOURCE_DIR .. "/ubo.hpp",
		ENGINE_SOURCE_DIR .. "/vertex.cpp",
		ENGINE_SOURCE_DIR .. "/vertex.hpp",
		ENGINE_SOURCE_DIR .. "/vpk.cpp",
		ENGINE_SOURCE_DIR .. "/vpk.hpp",
	}
	
	links {
		"SDL2main",
		"SDL2"
	}
	
	includedirs {
		VULKAN_INCLUDE_DIR
	}
	libdirs {
		VULKAN_LIBRARY_DIR
	}
	
	debugcommand( "%{GAME_DIR}/%{cfg.buildtarget.name}" )
	debugargs { "-game", "mod_mygame" }
	
	postbuildcommands {
		"{COPY} %{cfg.buildtarget.abspath} %{GAME_DIR}"
	}
	
	filter { "configurations:Release", "action:vs*" }
		links {
			"fmt",
			"assimp-vc142-mt"
		}
	
	filter { "configurations:Debug", "action:vs*" }
		links {
			"fmtd",
			"assimp-vc142-mtd"
		}
	
	filter { "action:not vs*" }
		links {
			"stdc++fs",
			"fmt",
			"assimp"
		}
	
	filter { "toolset:gcc", "system:windows" }
		links {
			"mingw32"
		}
	
	filter { "action:not vs*" }
		links { VULKAN_LIBRARY }
	
	filter { "action:vs*" }
		links { VC_VULKAN_LIBRARY }
	
	filter { "configurations:Debug", "system:windows" }
		debugdir "%{GAME_DIR}"
		postbuildcommands {
			"{COPY} %{DEPENDENCY_DIR}/x64-windows/debug/bin/assimp-vc142-mtd.dll %{GAME_DIR}",
			"{COPY} %{DEPENDENCY_DIR}/x64-windows/bin/SDL2.dll %{GAME_DIR}"
		}
		
	filter { "configurations:Release", "system:windows" }
		debugdir "%{GAME_DIR}"
		postbuildcommands {
			"{COPY} %{DEPENDENCY_DIR}/x64-windows/bin/assimp-vc142-mt.dll %{GAME_DIR}",
			"{COPY} %{DEPENDENCY_DIR}/x64-windows/bin/SDL2.dll %{GAME_DIR}"
		}
