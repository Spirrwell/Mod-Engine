#ifndef ENGINE_HPP
#define ENGINE_HPP

#include <typeindex>
#include <typeinfo>
#include <vector>
#include <unordered_map>

#include "SDL2/SDL.h"
#include "sdlwrapper.hpp"
#include "sdlwindow.hpp"

#include "memory.hpp"
#include "engine/iengine.hpp"
#include "enginesystem.hpp"
#include "commandlinesystem.hpp"
#include "filesystem.hpp"
#include "modulesystem.hpp"
#include "inputsystem.hpp"
#include "vulkansystem.hpp"
#include "texturesystem.hpp"
#include "shadersystem.hpp"
#include "materialsystem.hpp"
#include "renderer.hpp"
#include "modelsystem.hpp"

class ResourcePool;

class Engine : public IEngine
{
	template < typename T >
	T *CheckSystem( T *system ) const
	{
		if ( !isConfigurationStage )
			Error( "Cannot get engine system outside of configuration stage!" );

		if ( !system )
			Error( "Engine System does not exist!" );

		return system;
	}

	template < typename T >
	unique_ptr< T > MakeSystem()
	{
		unique_ptr< T > system = make_unique< T >();
		engineSystems.push_back( system.get() );

		return system;
	}

	void ConfigureEngineSystems()
	{
		for ( auto system : engineSystems )
			system->configure( this );
	}

	void UnconfigureEngineSystems()
	{
		// We want to unconfigure in reverse order!
		for ( auto it = engineSystems.rbegin(); it != engineSystems.rend(); ++it )
			( *it )->unconfigure( this );
	}

public:

	Engine( const int argc, const char * const *argv );

	[[noreturn]] void Error( const std::string_view msg ) const;

	void SetGame( IGame *game ) override { activeGame = game; }
	IGame *GetGame() const override { return activeGame; }

	void Init();
	void Shutdown();

	void Run();
	void Quit() { isRunning = false; }

	int GetArgC() const { return argc; }
	const char * const *GetArgV() const { return argv; }

	SDL_Window *GetWindow() const { return window->get(); }
	CommandLineSystem *GetCommandLineSystem() const { return CheckSystem( commandlineSystem.get() ); }
	FileSystem *GetFileSystem() const { return CheckSystem( fileSystem.get() ); }
	ModuleSystem *GetModuleSystem() const { return CheckSystem( moduleSystem.get() ); }
	VulkanSystem *GetVulkanSystem() const { return CheckSystem( vulkanSystem.get() ); }
	TextureSystem *GetTextureSystem() const { return CheckSystem( textureSystem.get() ); }
	ShaderSystem *GetShaderSystem() const { return CheckSystem( shaderSystem.get() ); }
	MaterialSystem *GetMaterialSystem() const { return CheckSystem( materialSystem.get() ); }
	Renderer *GetRenderer() const { return CheckSystem( renderer.get() ); }
	InputSystem *GetInputSystem() const { return CheckSystem( inputSystem.get() ); }
	ModelSystem *GetModelSystem() const { return CheckSystem( modelSystem.get() ); }

	IResourcePool *CreateResourcePool( IResourcePool *parent ) override;
	void DestroyResourcePool( IResourcePool *resourcePoolPtr ) override;

	IResourcePool *GetGlobalResourcePool() const override;

	IRenderer *GetIRenderer() const override { return GetRenderer(); }
	IInputSystem *GetIInputSystem() const override { return GetInputSystem(); }
	IModelSystem *GetIModelSystem() const override { return GetModelSystem(); }

	static Engine *CreateEngine( const int argc, const char * const *argv );
	static void DestroyEngine( Engine *engine );

protected:

	void WPrint( const std::wstring_view msg ) override;
	void WPrintln( const std::wstring_view msg ) override;

	void Print( const std::string_view msg ) override;
	void Println( const std::string_view msg ) override;

private:

	std::mutex logMutex;

	unique_ptr< SDLWrapper > sdl;
	unique_ptr< SDLWindow > window;

	unique_ptr< CommandLineSystem > commandlineSystem;
	unique_ptr< FileSystem > fileSystem;
	unique_ptr< ModuleSystem > moduleSystem;
	unique_ptr< VulkanSystem > vulkanSystem;
	unique_ptr< TextureSystem > textureSystem;
	unique_ptr< ShaderSystem > shaderSystem;
	unique_ptr< MaterialSystem > materialSystem;
	unique_ptr< Renderer > renderer;
	unique_ptr< InputSystem > inputSystem;
	unique_ptr< ModelSystem > modelSystem;
	std::vector< unique_ptr< ResourcePool > > resourcePools;

	ResourcePool *globalResourcePool = nullptr;
	std::vector< EngineSystem* > engineSystems;

	IGame *activeGame = nullptr;

	bool isRunning = false;
	bool isConfigurationStage = false;

	const int argc = 0;
	const char * const *argv;

	std::vector< IModel* > models;
};

#endif // ENGINE_HPP