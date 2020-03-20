#include "engine.hpp"
#include "clock.hpp"
#include "glm/gtx/transform.hpp"
#include "inputsystem.hpp"
#include "resourcepool.hpp"
#include "color.hpp"
#include "log.hpp"
#include "thread.hpp"
#include "game/igame.hpp"

#include <iostream>
#include <algorithm>

Engine::Engine( const int argc, const char * const *argv ) :
	argc( argc ),
	argv( argv )
{
}

void Engine::Error( const std::string_view msg ) const
{
	Log::PrintlnColor( fmt::color::red, msg );

	// Don't even bother showing messagebox if not on main thread for now
	if ( std::this_thread::get_id() == Thread::GetMainThreadId() )
		SDL_ShowSimpleMessageBox( SDL_MESSAGEBOX_ERROR, "System Error", fmt::format( "\n{}", msg ).data(), nullptr );

	SDL_TriggerBreakpoint();

	std::exit( EXIT_FAILURE );
}

void Engine::Init()
{
	Log::configure( this );

	isConfigurationStage = true;

	sdl = make_unique< SDLWrapper >( SDL_INIT_EVERYTHING );

	if ( !sdl->is_valid() )
		Error( fmt::format( "[SDL]{}", SDL_GetError() ) );

	window = make_unique< SDLWindow >( "ModEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN );

	if ( !window->get() )
		Error( fmt::format( "[SDL]{}", SDL_GetError() ) );

	SDL_SetWindowGrab( window->get(), SDL_TRUE );
	SDL_SetRelativeMouseMode( SDL_TRUE );

	commandlineSystem = MakeSystem< CommandLineSystem >();
	fileSystem = MakeSystem< FileSystem >();
	moduleSystem = MakeSystem< ModuleSystem >();
	vulkanSystem = MakeSystem< VulkanSystem >();
	textureSystem = MakeSystem< TextureSystem >();
	shaderSystem = MakeSystem< ShaderSystem >();
	materialSystem = MakeSystem< MaterialSystem >();
	renderer = MakeSystem< RenderSystem >();
	inputSystem = MakeSystem< InputSystem >();
	modelSystem = MakeSystem< ModelSystem >();

	// Create global resource pool for default assets to be loaded to
	globalResourcePool = ResourcePool::ToResourcePool( CreateResourcePool( nullptr ) );

	ConfigureEngineSystems();

	// Load default resources here
	textureSystem->LoadDefaultTextures();
	shaderSystem->LoadShaders();
	materialSystem->LoadDefaultMaterials();

	if ( !moduleSystem->LoadGameBinModule( "game" ) )
		Log::PrintlnRainbow( "Failed to load game module" );

	if ( activeGame )
		activeGame->configure( this );

	isConfigurationStage = false;
}

void Engine::Shutdown()
{
	if ( activeGame )
		activeGame->unconfigure( this );

	delete activeGame;

	moduleSystem->UnloadGameBinModule( "game" );

	resourcePools.clear();
	globalResourcePool = nullptr;

	UnconfigureEngineSystems();
	engineSystems.clear();

	Log::unconfigure();
}

void Engine::Run()
{
	isRunning = true;

	Clock delta;
	delta.Start();

	auto GetDeltaTime = [ &delta ]() -> float
	{
		float dt = delta.Duration();

		//if ( dt < 0.001f )
			//dt = 0.001f;

		return dt;
	};

	while( isRunning )
	{
		const float dt = GetDeltaTime();
		delta.Start();

		inputSystem->HandleSDLEvents();

		if ( activeGame )
			activeGame->tick( dt );

		renderer->BeginFrame();
		renderer->DrawScene();
		renderer->EndFrame();
	}
}

IResourcePool *Engine::CreateResourcePool( IResourcePool *parent )
{
	resourcePools.push_back( make_unique< ResourcePool >( parent ) );
	return resourcePools.back().get();
}

void Engine::DestroyResourcePool( IResourcePool *resourcePoolPtr )
{
	for ( auto it = resourcePools.begin(); it != resourcePools.end(); ++it )
	{
		if ( it->get() == resourcePoolPtr )
		{
			resourcePools.erase( it );
			break;
		}
	}
}

IResourcePool *Engine::GetGlobalResourcePool() const
{
	return globalResourcePool;
}

Engine *Engine::CreateEngine( const int argc, const char * const* argv )
{
	Engine *engine = new Engine( argc, argv );
	engine->Init();
	return engine;
}

void Engine::DestroyEngine( Engine *engine )
{
	engine->Shutdown();
	delete engine;
}

void Engine::WPrint( const std::wstring_view msg ) 
{
	std::lock_guard< std::mutex > lock( logMutex );
	std::wcout << msg;
}

void Engine::WPrintln( const std::wstring_view msg )
{
	std::lock_guard< std::mutex > lock( logMutex );
	std::wcout << msg << std::endl;
}

void Engine::Print( const std::string_view msg )
{
	std::lock_guard< std::mutex > lock( logMutex );
	std::cout << msg;
}

void Engine::Println( const std::string_view msg )
{
	std::lock_guard< std::mutex > lock( logMutex );
	std::cout << msg << std::endl;
}