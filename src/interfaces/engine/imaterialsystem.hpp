#ifndef IMATERIALSYSTEM_HPP
#define IMATERIALSYSTEM_HPP

#include <filesystem>
#include "engine/imaterial.hpp"
#include "engine/iresourcepool.hpp"

class IMaterialSystem
{
public:
	virtual ~IMaterialSystem() = default;

	// Loads material by relative path, GAME_DIR/models/relpath
	virtual IMaterial *LoadMaterial( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr ) = 0;
	virtual IMaterial *FindMaterial( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const = 0;
};

#endif // IMATERIALSYSTEM_HPP