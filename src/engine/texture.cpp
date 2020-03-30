#include "texture.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "rendersystem.hpp"

Texture::Texture( VulkanSystem *vulkanSystem ) :
	vulkanSystem( vulkanSystem )
{
}

Texture::~Texture()
{
	vulkanSystem->WaitIdle();

	if ( textureSampler != VK_NULL_HANDLE ) {
		vulkanSystem->DestroySampler( textureSampler, nullptr );
		textureSampler = VK_NULL_HANDLE;
	}

	if ( textureImageView != VK_NULL_HANDLE ) {
		vkDestroyImageView( vulkanSystem->device, textureImageView, nullptr );
		textureImageView = VK_NULL_HANDLE;
	}

	if ( textureImage != VK_NULL_HANDLE ) {
		vmaDestroyImage( vulkanSystem->allocator, textureImage, textureImageAllocation );
		textureImage = VK_NULL_HANDLE;
		textureImageAllocation = VK_NULL_HANDLE;
	}
}

void Texture::LoadRGBA( const unsigned char *pPixels, uint32_t width, uint32_t height, bool bGenMipMaps /*= false*/ )
{
	constexpr const uint32_t numChannels = 4;

	if ( bGenMipMaps ) {
		mipLevels = static_cast< uint32_t >( std::floor( std::log2( std::max( width, height ) ) ) ) + 1;
	}

	VkDeviceSize imageSize = ( VkDeviceSize )width * ( VkDeviceSize )height * ( VkDeviceSize )numChannels;

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

	vulkanSystem->VmaCreateBuffer( imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferAllocation );

	void *pData = nullptr;
	vulkanSystem->VmaMapMemory( stagingBufferAllocation, &pData );
		std::memcpy( pData, pPixels, static_cast< size_t >( imageSize ) );
	vulkanSystem->VmaUnmapMemory( stagingBufferAllocation );

	vulkanSystem->VmaCreateImage2D(
			width,
			height,
			mipLevels,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_TILING_OPTIMAL,
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VMA_MEMORY_USAGE_GPU_ONLY,
			textureImage,
			textureImageAllocation );

	vulkanSystem->TransitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, mipLevels );
	vulkanSystem->CopyBufferToImage( stagingBuffer, textureImage, width, height );

	// Generate Mipmaps
	if ( bGenMipMaps ) {
		vulkanSystem->GenerateMipMaps( textureImage, VK_FORMAT_R8G8B8A8_UNORM, width, height, mipLevels );
	}
	else {
		vulkanSystem->TransitionImageLayout( textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, mipLevels );
	}

	vulkanSystem->VmaDestroyBuffer( stagingBuffer, stagingBufferAllocation );

	// Create Texture Image View
	textureImageView = vulkanSystem->CreateImageView2D( textureImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, mipLevels );

	// Create Texture Sampler
	VkSamplerCreateInfo samplerInfo = {};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = 16.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = static_cast< float >( mipLevels );

	vulkanSystem->CreateSampler( &samplerInfo, nullptr, &textureSampler );
}