#ifndef VULKANSYSTEM_HPP
#define VULKANSYSTEM_HPP

#include <iostream>
#include <vector>
#include <array>
#include <optional>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "memory.hpp"
#include "enginesystem.hpp"

#define VK_DEBUG 1

class VulkanSystem;

static constexpr const auto MAX_FRAMES_IN_FLIGHT = 2;

struct vulkanContextCreateInfo
{
	VkPhysicalDeviceFeatures requiredPhysicalDeviceFeatures;
};

class VulkanInterface
{
public:
	VulkanInterface( VulkanSystem *vulkanSystem );
	virtual ~VulkanInterface();

	virtual void onSwapChainResize() = 0;

protected:
	VulkanSystem *vulkanSystem = nullptr;
};

class VulkanSystem : public EngineSystem
{
	std::vector< const char* > RequiredExtensions;

	struct QueueFamilyIndices
	{
		std::optional< uint32_t > graphicsFamily;
		std::optional< uint32_t > presentFamily;

		inline bool isComplete() const { return ( graphicsFamily.has_value() && presentFamily.has_value() ); }
	};

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities = {};
		std::vector< VkSurfaceFormatKHR > formats;
		std::vector< VkPresentModeKHR > presentModes;
	};

#if VK_DEBUG
	VkResult CreateDebugUtilsMessengerEXT ( const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator );
	void DestroyDebugUtilsMessengerEXT( VkAllocationCallbacks *pAllocator );
#endif

	// Helper Functions
	shared_ptr< SwapChainSupportDetails > QuerySwapChainSupport( VkPhysicalDevice device );
	QueueFamilyIndices FindQueueFamilies( VkPhysicalDevice device );
	bool IsDeviceSuitable( VkPhysicalDevice device );

	// Main initialization functions
	void LoadExtensions( SDL_Window *pWindow );
#if VK_DEBUG
	void CheckValidationLayerSupport();
#endif
	void CreateInstance();
#if VK_DEBUG
	void SetupDebugCallback();
#endif
	void CreateSurface( SDL_Window *pWindow );
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateVmaAllocator();
	void CreateSwapChain( const uint32_t width, const uint32_t height );
	void CreateImageViews();
	void FindDepthFormat();
	void CreateRenderPass();
	void CreateCommandPool();
	void CreateDepthResources();
	void CreateFramebuffers();
	void CreateSyncObjects();

	void DestroySwapChain();

public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	void WaitIdle();

	// Tells the VulkanSystem the window has been resized, re-creates the swap chain, returns false if there was an issue
	void NotifyWindowResized( uint32_t width, uint32_t height );

	void RegisterInterface( VulkanInterface *vulkanInterface );
	void UnregisterInterface( VulkanInterface *vulkanInterface );

	// These mirror vkCreate and vkDestroy functions to serve as wrappers for error handling
	void CreateShaderModule( const VkShaderModuleCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule );
	void DestroyShaderModule( VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator );
	void CreateGraphicsPipelines( VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines );
	void DestroyPipeline( VkPipeline pipeline, const VkAllocationCallbacks *pAllocator );
	void CreatePipelineLayout( const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout );
	void DestroyPipelineLayout( VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator );
	void CreateSampler( const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSampler *pSampler );
	void DestroySampler( VkSampler sampler, const VkAllocationCallbacks *pAllocator );
	void CreateDescriptorSetLayout( const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout );
	void DestroyDescriptorSetLayout( VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks *pAllocator );
	void CreateDescriptorPool( const VkDescriptorPoolCreateInfo *poolInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool );
	void DestroyDescriptorPool( VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator );
	void AllocateDescriptorSets( const VkDescriptorSetAllocateInfo *pAllocateInfo, VkDescriptorSet *pDescriptorSets );

	// OtherHelper functions
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands( VkCommandBuffer &commandBuffer );
	uint32_t FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties );
	void CopyBuffer( const VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize size );
	void VmaMapMemory( VmaAllocation allocation, void **ppData );
	void VmaUnmapMemory( VmaAllocation allocation );
	void VmaCreateBuffer( VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VkBuffer &buffer, VmaAllocation &allocation );
	void VmaDestroyBuffer( VkBuffer buffer, VmaAllocation allocation );
	VkImageView CreateImageView2D( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels );
	void VmaCreateImage2D( uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImage &image, VmaAllocation &allocation );
	void CopyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height );
	void TransitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels );
	void GenerateMipMaps( VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels );

	std::array< const char*, 1 > deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkInstance instance = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT debugCallback = VK_NULL_HANDLE;
	std::vector< const char* > validationLayers = { "VK_LAYER_LUNARG_standard_validation" };
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	QueueFamilyIndices queueFamilyIndices;
	VkDevice device = VK_NULL_HANDLE;
	VmaAllocator allocator = VK_NULL_HANDLE;
	VkQueue graphicsQueue = VK_NULL_HANDLE;
	VkQueue presentQueue = VK_NULL_HANDLE;
	VkCommandPool commandPool = VK_NULL_HANDLE;

	VkFormat swapChainImageFormat = VK_FORMAT_UNDEFINED;
	VkExtent2D swapChainExtent = {};
	VkSwapchainKHR swapChainKHR = VK_NULL_HANDLE;
	unique_ptr< VkImage[] > swapChainImages;
	unique_ptr< VkImageView[] > swapChainImageViews;
	unique_ptr< VkFramebuffer[] > swapChainFramebuffers;
	VkFormat depthFormat = VK_FORMAT_UNDEFINED;
	VkRenderPass renderPass = VK_NULL_HANDLE;
	VkImage depthImage = VK_NULL_HANDLE;
	VmaAllocation depthImageAllocation = VK_NULL_HANDLE;
	VkImageView depthImageView = VK_NULL_HANDLE;
	uint32_t numSwapChainImages = 0;

	VkSemaphore imageAvailableSemaphores[ MAX_FRAMES_IN_FLIGHT ] = { VK_NULL_HANDLE };
	VkSemaphore renderFinishedSemaphores[ MAX_FRAMES_IN_FLIGHT ] = { VK_NULL_HANDLE };
	VkFence inFlightFences[ MAX_FRAMES_IN_FLIGHT ] = { VK_NULL_HANDLE };

	std::vector< VulkanInterface* > vulkanInterfaces;
};

#endif // VULKANSYSTEM_HPP