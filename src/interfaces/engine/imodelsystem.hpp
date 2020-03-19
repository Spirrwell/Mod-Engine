#ifndef IMODELSYSTEM_HPP
#define IMODELSYSTEM_HPP

#include <limits>
#include <cstdint>
#include <filesystem>

#include "memory.hpp"
#include "engine/imodel.hpp"
#include "engine/iresourcepool.hpp"

class IModelSystem
{
public:
	virtual ~IModelSystem() = default;

	// Loads model by relative path, GAME_DIR/models/relpath
	virtual IModel *LoadModel( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr ) = 0;
	virtual IModel *FindModel( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const = 0;
};

#endif // IMODELSYSTEM_HPP