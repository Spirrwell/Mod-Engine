#include "texturesystem.hpp"
#include "texture.hpp"
#include "log.hpp"
#include "stb_image.h"
#include "stb_filesystem.hpp"
#include "vfile.hpp"
#include "engine.hpp"
#include "resourcepool.hpp"

void TextureSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	fileSystem = engine->GetFileSystem();
	vulkanSystem = engine->GetVulkanSystem();
}

void TextureSystem::unconfigure( Engine *engine )
{
	fileSystem = nullptr;
	vulkanSystem = nullptr;

	EngineSystem::unconfigure( engine );
}

void TextureSystem::LoadDefaultTextures()
{
	errorTexture = Texture::ToTexture( LoadTexture( "textures/error.png", "GAME", engine->GetGlobalResourcePool() ) );

	if ( !errorTexture )
		engine->Error( "Failed to load error texture!" );
}

ITexture *TextureSystem::LoadTexture( const std::filesystem::path &relpath, const std::string &pathid, IResourcePool *resourcePoolPtr )
{
	ResourcePool *resourcePool = ResourcePool::ToResourcePool( resourcePoolPtr );
	
	if ( !resourcePool )
	{
		Log::PrintlnWarn( "[TextureSystem]Resource pool is NULL" );
		return errorTexture;
	}

	if ( ITexture *texture = FindTexture_Internal( relpath, resourcePoolPtr ); texture )
		return texture;

	VFile file( relpath, pathid, fileSystem );

	if ( !file.is_open() )
		return errorTexture;

	int x = 0;
	int y = 0;
	int numComponents = 0;

	stbi_uc *pixels = stbi_load_from_callbacks( &callbacks_stb, &file, &x, &y, &numComponents, STBI_rgb_alpha );

	if ( pixels == nullptr ) {
		// Don't use stbi_failure_reason because it's sadly not thread-safe
		Log::PrintlnWarn( "Failed to load texture {}", relpath.generic_string() );
		return errorTexture;
	}

	auto resource = ResourcePool::createResource< Texture >( ResourceInfo{ relpath.generic_string() }, vulkanSystem );
	Texture *texture = resource->resource.get();
	texture->LoadRGBA( pixels, x, y, true );

	stbi_image_free( pixels );

	texturesMutex.lock();
	resourcePool->textures.push_back( resource );
	texturesMutex.unlock();

	return texture;
}

ITexture *TextureSystem::FindTexture( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const
{
	if ( ResourcePool *resourcePool = ResourcePool::ToResourcePool( resourcePoolPtr ); resourcePool )
		return resourcePool->findResource< Texture >( relpath.generic_string() );

	return nullptr;
}

ITexture *TextureSystem::FindTexture_Internal( const std::filesystem::path &relpath, IResourcePool *resourcePoolPtr ) const
{
	// We only need to lock guard our own lookups
	std::lock_guard< std::mutex > lock( texturesMutex );
	return FindTexture( relpath, resourcePoolPtr );
}