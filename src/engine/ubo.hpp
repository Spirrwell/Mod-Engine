#ifndef UBO_HPP
#define UBO_HPP

#include "memory.hpp"
#include "vulkansystem.hpp"

class UBO
{
public:
	UBO( VulkanSystem *vulkanSystem, VkDeviceSize bufferSize );
	~UBO();

	VkDeviceSize BufferSize() const { return bufferSize; }

	std::vector< VkBuffer > uniformBuffer; // Buffer per swap chain image
	std::vector< VmaAllocation > uniformBufferAllocation; // Buffer allocation per swap chain image

private:
	VulkanSystem *vulkanSystem = nullptr;
	VkDeviceSize bufferSize;
};

#endif // UBO_HPP