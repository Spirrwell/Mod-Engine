#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include "engine/itexture.hpp"
#include "vulkansystem.hpp"
#include "memory.hpp"
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

class Renderer;

class Texture : public ITexture
{
public:
	Texture( VulkanSystem *vulkanSystem );
	~Texture();

	static inline Texture *ToTexture( ITexture *texture ) { return static_cast< Texture* >( texture ); }

	void LoadRGBA( const unsigned char *pPixels, uint32_t width, uint32_t height, bool bGenMipMaps = false );

	const VkImageView GetImageView() const { return textureImageView; }
	const VkSampler GetSampler() const { return textureSampler; }

	VkImage textureImage = VK_NULL_HANDLE;
	VmaAllocation textureImageAllocation = VK_NULL_HANDLE;
	VkImageView textureImageView = VK_NULL_HANDLE;
	VkSampler textureSampler = VK_NULL_HANDLE;
	uint32_t mipLevels = 1;

	VulkanSystem *vulkanSystem = nullptr;
};

#endif // TEXTURE_HPP