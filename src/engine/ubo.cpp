#include "ubo.hpp"

UBO::UBO( VulkanSystem *vulkanSystem, VkDeviceSize bufferSize ) :
	vulkanSystem( vulkanSystem ),
	bufferSize( bufferSize )
{
	uniformBuffer.resize( vulkanSystem->numSwapChainImages );
	uniformBufferAllocation.resize( vulkanSystem->numSwapChainImages );

	for ( uint32_t i = 0; i < vulkanSystem->numSwapChainImages; ++i ) {
		vulkanSystem->VmaCreateBuffer( bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, uniformBuffer[ i ], uniformBufferAllocation[ i ] );
	}
}

UBO::~UBO()
{
	if ( uniformBuffer.size() > 0 ) {
		for ( uint32_t i = 0; i < vulkanSystem->numSwapChainImages; ++i ) {
			vulkanSystem->VmaDestroyBuffer( uniformBuffer[ i ], uniformBufferAllocation[ i ] );
		}

		uniformBuffer.clear();
		uniformBufferAllocation.clear();
	}
}