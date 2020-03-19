#ifndef ITEXTURESYSTEM_HPP
#define ITEXTURESYSTEM_HPP

#include "engine/itexture.hpp"
#include "engine/iresourcepool.hpp"

#include <string>
#include <filesystem>

class ITextureSystem
{
public:
	virtual ~ITextureSystem() = default;

	virtual ITexture *LoadTexture( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr ) = 0;
	virtual ITexture *FindTexture( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const = 0;
};

#endif // ITEXTURESYSTEM_HPP