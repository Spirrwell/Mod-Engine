#ifndef TEXTURESYSTEM_HPP
#define TEXTURESYSTEM_HPP

#include <unordered_map>
#include <string>
#include <mutex>

#include "engine/itexturesystem.hpp"
#include "enginesystem.hpp"
#include "filesystem.hpp"
#include "memory.hpp"
#include "vulkansystem.hpp"

class Texture;

class TextureSystem : public ITextureSystem, public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	void LoadDefaultTextures();

	ITexture *LoadTexture( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr ) override;
	ITexture *FindTexture( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const override;

private:
	ITexture *FindTexture_Internal( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const;

public:

	Texture *GetErrorTexture() const { return errorTexture; }

private:

	FileSystem *fileSystem = nullptr;
	VulkanSystem *vulkanSystem = nullptr;

	Texture *errorTexture = nullptr;
	mutable std::mutex texturesMutex;
};

#endif // TEXTURESYSTEM_HPP