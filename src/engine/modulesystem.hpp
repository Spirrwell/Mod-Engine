#ifndef MODULESYSTEM_HPP
#define MODULESYSTEM_HPP

#include "engine/iengine.hpp"
#include "enginesystem.hpp"
#include "filesystem.hpp"

#include <string>
#include <vector>

struct Module
{
	void (*module_configure)( IEngine *engine ) = nullptr;
	void (*module_unconfigure)( IEngine *engine ) = nullptr;
	std::string moduleName;
	void *pModule = nullptr;
};

class ModuleSystem : public EngineSystem
{
public:
	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	bool LoadGameBinModule( const std::filesystem::path &relpath );
	void UnloadGameBinModule( const std::filesystem::path &relpath );

private:
	std::filesystem::path MakeGameBinPath( const std::filesystem::path &relpath );

	void UnloadModule( Module &module );
	bool IsModuleLoaded( const std::filesystem::path &abspath ) const;

	FileSystem *fileSystem = nullptr;
	std::filesystem::path gameBinDir;

	std::vector< Module > modules;
};

#endif // MODULESYSTEM_HPP