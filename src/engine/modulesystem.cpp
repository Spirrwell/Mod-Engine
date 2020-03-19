#include "modulesystem.hpp"
#include "engine.hpp"

#include <SDL2/SDL.h>

#if defined( _WIN32 )
static const std::filesystem::path MODULE_EXT = ".dll";
#elif defined( __linux__ )
static const std::filesystem::path MODULE_EXT = ".so";
#endif

void ModuleSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	fileSystem = engine->GetFileSystem();
	gameBinDir = fileSystem->GetGameBinDir();
}

void ModuleSystem::unconfigure( Engine *engine )
{
	for ( auto it = modules.rbegin(); it != modules.rend(); ++it )
		UnloadModule( *it );

	modules.clear();

	fileSystem = nullptr;
	gameBinDir.clear();

	EngineSystem::unconfigure( engine );
}

bool ModuleSystem::LoadGameBinModule( const std::filesystem::path &relpath )
{
	const std::filesystem::path abspath = MakeGameBinPath( relpath );
	Log::PrintlnRainbow( "Loading module {}", abspath.generic_string() );

	if ( IsModuleLoaded( abspath ) )
		return true;

	Module module;
	module.pModule = SDL_LoadObject( abspath.generic_string().c_str() );

	if ( !module.pModule )
	{
		Log::PrintlnWarn( "Failed to load module {}", abspath.generic_string() );
		return false;
	}

	module.module_configure = (void (*)(IEngine*))SDL_LoadFunction( module.pModule, "module_configure" );

	if ( !module.module_configure )
	{
		Log::PrintlnWarn( "Failed to load module_configure function for module {}", abspath.generic_string() );
		SDL_UnloadObject( module.pModule );
		return false;
	}

	module.module_unconfigure = (void (*)(IEngine*))SDL_LoadFunction( module.pModule, "module_unconfigure" );

	if ( !module.module_unconfigure )
	{
		Log::PrintlnWarn( "Failed to load module_unconfigure function for module {}", abspath.generic_string() );
		SDL_UnloadObject( module.pModule );
		return false;
	}

	module.module_configure( engine );

	modules.push_back(
		module
	);

	return true;
}

void ModuleSystem::UnloadGameBinModule( const std::filesystem::path &relpath )
{
	const std::filesystem::path abspath = MakeGameBinPath( relpath );
	const std::string abspath_string = abspath.generic_string();

	for ( auto it = modules.begin(); it != modules.end(); ++it )
	{
		if ( it->moduleName == abspath_string )
		{
			UnloadModule( *it );
			modules.erase( it );
			break;
		}
	}
}

std::filesystem::path ModuleSystem::MakeGameBinPath( const std::filesystem::path &relpath )
{
	std::filesystem::path abspath = gameBinDir / relpath;

	if ( abspath.has_extension() )
		abspath.replace_extension( MODULE_EXT );
	else
		abspath += MODULE_EXT;

	return abspath;
}

void ModuleSystem::UnloadModule( Module &module )
{
	module.module_unconfigure( engine );
	SDL_UnloadObject( module.pModule );
}

bool ModuleSystem::IsModuleLoaded( const std::filesystem::path &abspath ) const
{
	const std::string abspath_string = abspath.generic_string();
	for ( auto &binModule : modules )
	{
		if ( binModule.moduleName == abspath_string )
			return true;
	}

	return false;
}