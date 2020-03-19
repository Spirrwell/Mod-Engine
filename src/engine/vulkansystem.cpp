#define VMA_IMPLEMENTATION // for vk_mem_alloc.h included in vulkansystem.hpp
#include "vulkansystem.hpp"
#include "log.hpp"
#include "engine.hpp"

#include <map>
#include <set>
#include <exception>

VulkanInterface::VulkanInterface( VulkanSystem *vulkanSystem ) : vulkanSystem( vulkanSystem )
{
	vulkanSystem->RegisterInterface( this );
}

VulkanInterface::~VulkanInterface()
{
	vulkanSystem->UnregisterInterface( this );
}

#if VK_DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback( VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData )
{
	Log::PrintlnWarn( "[Vulkan]Validation layer: {}", pCallbackData->pMessage );
	return VK_FALSE;
}
#endif // VK_DEBUG

void VulkanSystem::LoadExtensions( SDL_Window *pWindow )
{
	unsigned int extensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions( pWindow, &extensionCount, nullptr );

	if ( extensionCount == 0 ) {
		engine->Error( "[Vulkan]Failed to load required extensions" );
	}

	RequiredExtensions.resize( static_cast< size_t >( extensionCount ) );
	SDL_Vulkan_GetInstanceExtensions( pWindow, &extensionCount, RequiredExtensions.data() );

#if VK_DEBUG
	RequiredExtensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
#endif

	Log::Println( "[Vulkan]Required Extensions:" );

	for ( auto &extension : RequiredExtensions )
		Log::Println( "\t{}", extension );
}

#if VK_DEBUG
void VulkanSystem::CheckValidationLayerSupport()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties( &layerCount, nullptr );

	std::vector< VkLayerProperties > availableLayers( static_cast< size_t >( layerCount ) );
	vkEnumerateInstanceLayerProperties( &layerCount, availableLayers.data() );

	for ( auto layerName : validationLayers )
	{
		bool bLayerFound = false;

		for ( const auto &layerProperties : availableLayers )
		{
			if ( std::strcmp( layerName, layerProperties.layerName ) == 0 ) {
				bLayerFound = true;
				break;
			}
		}

		if ( !bLayerFound ) {
			engine->Error( "[Vulkan]Failed to acquire validation layers" );
		}
	}
}
#endif // VK_DEBUG

void VulkanSystem::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "quickvulkan";
	appInfo.applicationVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.pEngineName = "quickvulkan";
	appInfo.engineVersion = VK_MAKE_VERSION( 1, 0, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast< uint32_t >( RequiredExtensions.size() );
	createInfo.ppEnabledExtensionNames = RequiredExtensions.data();

#if VK_DEBUG
	createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
	createInfo.ppEnabledLayerNames = validationLayers.data();
#else // !VK_DEBUG
	createInfo.enabledLayerCount = 0;
#endif // VK_DEBUG

	if ( vkCreateInstance( &createInfo, nullptr, &instance ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create instance" );
	}

	// Query available extensions
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, nullptr );

	std::vector< VkExtensionProperties > extensionProperties{ static_cast< size_t >( extensionCount ) };
	vkEnumerateInstanceExtensionProperties( nullptr, &extensionCount, extensionProperties.data() );

	Log::Println( "[Vulkan]Available Extensions:" );

	for ( const auto &extensionProperty : extensionProperties )
		Log::Println( "\t{}", extensionProperty.extensionName );

	Log::Println( "" );
}

#if VK_DEBUG
void VulkanSystem::SetupDebugCallback()
{
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;

	if ( CreateDebugUtilsMessengerEXT( &createInfo, nullptr ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to setup debug callback" );
	}
}
#endif

void VulkanSystem::CreateSurface( SDL_Window *pWindow )
{
	if ( SDL_Vulkan_CreateSurface( pWindow, instance, &surface ) != SDL_TRUE ) {
		engine->Error( "[Vulkan]Failed to create window surface" );
	}
}

void VulkanSystem::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices( instance, &deviceCount, nullptr );

	if ( deviceCount == 0 ) {
		engine->Error( "[Vulkan]Failed to find physical device" );
	}

	std::vector< VkPhysicalDevice > devices( static_cast< size_t >( deviceCount ) );
	vkEnumeratePhysicalDevices( instance, &deviceCount, devices.data() );

	// TODO: Allow for supplying custom physical device rating callback
	auto rateSuitability = []( VkPhysicalDevice device )
	{
		uint32_t iScore = 1;

		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties( device, &deviceProperties );

		VkPhysicalDeviceFeatures deviceFeatures;
		vkGetPhysicalDeviceFeatures( device, &deviceFeatures );

		if ( deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU ) {
			iScore += 1000;
		}

		iScore += deviceProperties.limits.maxImageDimension2D;

		return iScore;
	};

	// Ordered map that automatically sorts device candidates by increasing score
	std::multimap< uint32_t, VkPhysicalDevice > candidates;

	for ( VkPhysicalDevice &device : devices )
	{
		if ( !IsDeviceSuitable( device ) ) {
			continue;
		}

		uint32_t score = rateSuitability( device );
		candidates.insert( std::make_pair( score, device ) );
	}

	if ( !candidates.empty() && candidates.rbegin()->first > 0 ) {
		physicalDevice = candidates.rbegin()->second;
	}

	if ( !physicalDevice ) {
		engine->Error( "[Vulkan]Failed to find physical device" );
	}

	queueFamilyIndices = FindQueueFamilies( physicalDevice );
}

void VulkanSystem::CreateLogicalDevice()
{
	std::vector< VkDeviceQueueCreateInfo > queueCreateInfos;
	std::set< uint32_t > uniqueQueueFamilies = { queueFamilyIndices.graphicsFamily.value(), queueFamilyIndices.presentFamily.value() };

	const float queuePriority = 1.0f;
	for ( auto &queueFamily : uniqueQueueFamilies )
	{
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back( queueCreateInfo );
	}

	// TODO: Revisit this in the future for requesting device features
	VkPhysicalDeviceFeatures deviceFeatures = {};
	vkGetPhysicalDeviceFeatures( physicalDevice, &deviceFeatures );
	//deviceFeatures.textureCompressionASTC_LDR = VK_TRUE;
	//deviceFeatures.textureCompressionETC2 = VK_TRUE;
	//deviceFeatures.samplerAnisotropy = VK_TRUE;
	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast< uint32_t >( queueCreateInfos.size() );
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast< uint32_t >( deviceExtensions.size() );
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#if VK_DEBUG
	createInfo.enabledLayerCount = static_cast< uint32_t >( validationLayers.size() );
	createInfo.ppEnabledLayerNames = validationLayers.data();
#else // !VK_DEBUG
	createInfo.enabledLayerCount = 0;
#endif // VK_DEBUG

	if ( vkCreateDevice( physicalDevice, &createInfo, nullptr, &device ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create logical device" );
	}

	vkGetDeviceQueue( device, queueFamilyIndices.graphicsFamily.value(), 0, &graphicsQueue );
	vkGetDeviceQueue( device, queueFamilyIndices.presentFamily.value(), 0, &presentQueue );
}

void VulkanSystem::CreateVmaAllocator()
{
	VmaAllocatorCreateInfo allocatorInfo = {};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;

	vmaCreateAllocator( &allocatorInfo, &allocator );
}

void VulkanSystem::CreateSwapChain( const uint32_t width, const uint32_t height )
{
	auto chooseSwapSurfaceFormat = []( const std::vector< VkSurfaceFormatKHR > &availableFormats )
	{
		if ( availableFormats.size() == 1 && availableFormats[ 0 ].format == VK_FORMAT_UNDEFINED ) {
			return VkSurfaceFormatKHR{ VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };
		}

		for ( const auto &format : availableFormats )
		{
			if ( format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR ) {
				return format;
			}
		}

		return availableFormats[ 0 ];
	};

	// TODO: Revisit this in the future for VSync options
	auto chooseSwapPresentMode = []( const std::vector< VkPresentModeKHR > &availablePresentModes )
	{
		auto selectedMode = VK_PRESENT_MODE_FIFO_KHR;

		for ( const auto &presentMode : availablePresentModes )
		{
			if ( presentMode == VK_PRESENT_MODE_MAILBOX_KHR ) {
				return presentMode;
			}
			else if ( presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR ) {
				selectedMode = presentMode;
			}
		}

		return selectedMode;
	};

	auto chooseSwapExtent = [ &width, &height, this ]( const VkSurfaceCapabilitiesKHR &capabilities )
	{
		if ( width == 0 || height == 0 ) {
			engine->Error( "Window created with size of 0!" );
		}

		VkExtent2D actualExtent = { width, height };

		actualExtent.width = std::clamp( actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width );
		actualExtent.height = std::clamp( actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height );

		if ( actualExtent.width == 0 || actualExtent.height == 0 ) {
			engine->Error( "[Vulkan]Swap chain capabilities clamped extent to 0!" );
		}

		return actualExtent;
	};

	auto swapChainSupport = QuerySwapChainSupport( physicalDevice );
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat( swapChainSupport->formats );
	VkPresentModeKHR presentMode = chooseSwapPresentMode( swapChainSupport->presentModes );
	VkExtent2D extent = chooseSwapExtent( swapChainSupport->capabilities );

	uint32_t minImageCount = swapChainSupport->capabilities.maxImageCount > 0 ?
		std::min( swapChainSupport->capabilities.minImageCount + 1, swapChainSupport->capabilities.maxImageCount ) :
		swapChainSupport->capabilities.minImageCount + 1;

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = minImageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	const uint32_t indices[] =
	{
		queueFamilyIndices.graphicsFamily.value(),
		queueFamilyIndices.presentFamily.value()
	};

	if ( indices[ 0 ] != indices[ 1 ] ) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}

	createInfo.preTransform = swapChainSupport->capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; // TODO: Revisit this for window blending
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = nullptr;

	if ( vkCreateSwapchainKHR( device, &createInfo, nullptr, &swapChainKHR ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create swap chain" );
	}

	// Get the actual number of swap chain images we will use
	vkGetSwapchainImagesKHR( device, swapChainKHR, &numSwapChainImages, nullptr );

	// Allocate our elements dependant on the number of swap chain images
	swapChainImages = make_unique< VkImage[] >( numSwapChainImages );
	swapChainImageViews = make_unique< VkImageView[] >( numSwapChainImages );
	swapChainFramebuffers = make_unique< VkFramebuffer[] >( numSwapChainImages );

	// Initialize everything to VK_NULL_HANDLE
	for ( uint32_t i = 0; i < numSwapChainImages; ++i )
	{
		swapChainImages[ i ] = VK_NULL_HANDLE;
		swapChainImageViews[ i ] = VK_NULL_HANDLE;
		swapChainFramebuffers[ i ] = VK_NULL_HANDLE;
	}

	vkGetSwapchainImagesKHR( device, swapChainKHR, &numSwapChainImages, &swapChainImages[ 0 ] );

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
}

void VulkanSystem::CreateImageViews()
{
	for ( uint32_t i = 0; i < numSwapChainImages; ++i )
		swapChainImageViews[ i ] = CreateImageView2D( swapChainImages[ i ], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1 );
}

void VulkanSystem::FindDepthFormat()
{
	auto findSupportedFormat = [ this ]( const std::vector< VkFormat > &candidates, VkImageTiling tiling, VkFormatFeatureFlags features )
	{
		for ( const auto &format : candidates )
		{
			VkFormatProperties props = {};
			vkGetPhysicalDeviceFormatProperties( physicalDevice, format, &props );

			if ( tiling == VK_IMAGE_TILING_LINEAR && ( props.linearTilingFeatures & features ) == features ) {
				return format;
			}
			else if ( tiling == VK_IMAGE_TILING_OPTIMAL && ( props.optimalTilingFeatures & features ) == features ) {
				return format;
			}
		}

		return VK_FORMAT_UNDEFINED;
	};

	depthFormat = findSupportedFormat(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
	);

	if ( depthFormat == VK_FORMAT_UNDEFINED ) {
		engine->Error( "[Vulkan]Failed to find supported depth format" );
	}
}

void VulkanSystem::CreateRenderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef;
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	std::array< VkAttachmentDescription, 2 > attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if ( vkCreateRenderPass( device, &renderPassInfo, nullptr, &renderPass ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create render pass" );
	}
}

void VulkanSystem::CreateCommandPool()
{
	auto queueFamilyIndices = FindQueueFamilies( physicalDevice );

	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	if ( vkCreateCommandPool( device, &poolInfo, nullptr, &commandPool ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create command pool" );
	}
}

void VulkanSystem::CreateDepthResources()
{
	VmaCreateImage2D( swapChainExtent.width, swapChainExtent.height, 1, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY, depthImage, depthImageAllocation );
	depthImageView = CreateImageView2D( depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, 1 );
	TransitionImageLayout( depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, 1 );
}

void VulkanSystem::CreateFramebuffers()
{
	for ( uint32_t i = 0; i < numSwapChainImages; ++i )
	{
		std::array< VkImageView, 2 > attachments = { swapChainImageViews[ i ], depthImageView };

		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast< uint32_t >( attachments.size() );
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if ( vkCreateFramebuffer( device, &framebufferInfo, nullptr, &swapChainFramebuffers[ i ] ) != VK_SUCCESS ) {
			engine->Error( "[Vulkan]Failed to create framebuffers" );
		}
	}
}

void VulkanSystem::CreateSyncObjects()
{
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for ( auto i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
	{
		if ( vkCreateSemaphore( device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[ i ] ) != VK_SUCCESS ) {
			engine->Error( "[Vulkan]Failed to create sync objects" );
		}

		if ( vkCreateSemaphore( device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[ i ] ) != VK_SUCCESS ) {
			engine->Error( "[Vulkan]Failed to create sync objects" );
		}

		if ( vkCreateFence( device, &fenceInfo, nullptr, &inFlightFences[ i ] ) != VK_SUCCESS ) {
			engine->Error( "[Vulkan]Failed to create sync objects" );
		}
	}
}

#if VK_DEBUG
VkResult VulkanSystem::CreateDebugUtilsMessengerEXT ( const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator )
{
	auto func = ( PFN_vkCreateDebugUtilsMessengerEXT )vkGetInstanceProcAddr( instance, "vkCreateDebugUtilsMessengerEXT" );
	return ( func ) ? func( instance, pCreateInfo, pAllocator, &debugCallback ) : VK_ERROR_EXTENSION_NOT_PRESENT;
}

void VulkanSystem::DestroyDebugUtilsMessengerEXT( VkAllocationCallbacks *pAllocator )
{
	auto func = ( PFN_vkDestroyDebugUtilsMessengerEXT ) vkGetInstanceProcAddr( instance, "vkDestroyDebugUtilsMessengerEXT" );

	if ( func != nullptr ) {
		func( instance, debugCallback, pAllocator );
	}
}
#endif

shared_ptr< VulkanSystem::SwapChainSupportDetails > VulkanSystem::QuerySwapChainSupport( VkPhysicalDevice device )
{
	shared_ptr< SwapChainSupportDetails > details = make_shared< SwapChainSupportDetails >();
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR( device, surface, &details->capabilities );

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &formatCount, nullptr );

	if ( formatCount != 0 ) {
		details->formats.resize( static_cast< size_t >( formatCount ) );
		vkGetPhysicalDeviceSurfaceFormatsKHR( device, surface, &formatCount, details->formats.data() );
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, nullptr );

	if ( presentModeCount != 0 ) {
		details->presentModes.resize( static_cast< size_t >( presentModeCount ) );
		vkGetPhysicalDeviceSurfacePresentModesKHR( device, surface, &presentModeCount, details->presentModes.data() );
	}

	return details;
}

VulkanSystem::QueueFamilyIndices VulkanSystem::FindQueueFamilies( VkPhysicalDevice device )
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, nullptr );

	std::vector< VkQueueFamilyProperties > queueFamilies( static_cast< size_t >( queueFamilyCount ) );
	vkGetPhysicalDeviceQueueFamilyProperties( device, &queueFamilyCount, queueFamilies.data() );

	for ( size_t index = 0; index < queueFamilies.size(); ++index )
	{
		if ( queueFamilies[ index ].queueCount > 0 ) {
			if ( queueFamilies[ index ].queueFlags & VK_QUEUE_GRAPHICS_BIT ) {
				indices.graphicsFamily = static_cast< uint32_t >( index );
			}

			VkBool32 presentSupport = VK_FALSE;
			vkGetPhysicalDeviceSurfaceSupportKHR( device, static_cast< uint32_t >( index ), surface, &presentSupport );

			if ( presentSupport == VK_TRUE ) {
				indices.presentFamily = static_cast< uint32_t >( index );
			}

			if ( indices.isComplete() ) {
				break;
			}
		}
	}

	return indices;
}

bool VulkanSystem::IsDeviceSuitable( VkPhysicalDevice device )
{
	auto checkDeviceExtensionSupport = [ device, this ]()
	{
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, nullptr );

		std::vector< VkExtensionProperties > availableExtensions( static_cast< size_t >( extensionCount ) );
		vkEnumerateDeviceExtensionProperties( device, nullptr, &extensionCount, availableExtensions.data() );

		std::set< std::string > requiredDeviceExtensionSet( deviceExtensions.begin(), deviceExtensions.end() );

		for ( const auto &extension : availableExtensions )
			requiredDeviceExtensionSet.erase( extension.extensionName );

		return ( requiredDeviceExtensionSet.empty() );
	};

	auto checkSwapChainSupport = [ & ]( shared_ptr< SwapChainSupportDetails > swapChainSupport )
	{
		return ( !swapChainSupport->formats.empty() && !swapChainSupport->presentModes.empty() );
	};

	// TODO: Allow custom callbacks for checking device features and extensions


	return ( FindQueueFamilies( device ).isComplete() &&
		checkDeviceExtensionSupport() &&
		checkSwapChainSupport( QuerySwapChainSupport( device ) ) );
}

void VulkanSystem::DestroySwapChain()
{
	if ( depthImageView != VK_NULL_HANDLE ) {
		vkDestroyImageView( device, depthImageView, nullptr );
		depthImageView = VK_NULL_HANDLE;
	}

	if ( depthImage != VK_NULL_HANDLE ) {
		vmaDestroyImage( allocator, depthImage, depthImageAllocation );
		depthImage = VK_NULL_HANDLE;
		depthImageAllocation = VK_NULL_HANDLE;
	}

	for ( uint32_t i = 0; i < numSwapChainImages; ++i )
	{
		if ( swapChainFramebuffers && swapChainFramebuffers[ i ] != VK_NULL_HANDLE ) {
			vkDestroyFramebuffer( device, swapChainFramebuffers[ i ], nullptr );
		}

		if ( swapChainImageViews && swapChainImageViews[ i ] != VK_NULL_HANDLE ) {
			vkDestroyImageView( device, swapChainImageViews[ i ], nullptr );
		}
	}

	swapChainFramebuffers.reset();
	swapChainImageViews.reset();
	swapChainImages.reset();

	if ( renderPass != VK_NULL_HANDLE ) {
		vkDestroyRenderPass( device, renderPass, nullptr );
		renderPass = VK_NULL_HANDLE;
	}

	if ( swapChainKHR != VK_NULL_HANDLE ) {
		vkDestroySwapchainKHR( device, swapChainKHR, nullptr );
		swapChainKHR = VK_NULL_HANDLE;
	}
}

void VulkanSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	LoadExtensions( engine->GetWindow() );
#if VK_DEBUG
	CheckValidationLayerSupport();
#endif // VK_DEUBG
	CreateInstance();
#if VK_DEBUG
	SetupDebugCallback();
#endif // VK_DEBUG
	CreateSurface( engine->GetWindow() );
	PickPhysicalDevice();
	CreateLogicalDevice();
	CreateVmaAllocator();

	int width = 0;
	int height = 0;

	SDL_GetWindowSize( engine->GetWindow(), &width, &height );

	CreateSwapChain( static_cast< uint32_t >( width ), static_cast< uint32_t >( height ) );
	CreateImageViews();
	FindDepthFormat();
	CreateRenderPass();
	CreateCommandPool();
	CreateDepthResources();
	CreateFramebuffers();
	CreateSyncObjects();
}

void VulkanSystem::unconfigure( Engine *engine )
{
	// Must wait for device to be idle before we can cleanup
	WaitIdle();

	// Clean up swap chain
	DestroySwapChain();

	// Destroy sync objects
	for ( size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i )
	{
		if ( renderFinishedSemaphores[ i ] != VK_NULL_HANDLE ) {
			vkDestroySemaphore( device, renderFinishedSemaphores[ i ], nullptr );
			renderFinishedSemaphores[ i ] = VK_NULL_HANDLE;
		}

		if ( imageAvailableSemaphores[ i ] != VK_NULL_HANDLE ) {
			vkDestroySemaphore( device, imageAvailableSemaphores[ i ], nullptr );
			imageAvailableSemaphores[ i ] = VK_NULL_HANDLE;
		}

		if ( inFlightFences[ i ] != VK_NULL_HANDLE ) {
			vkDestroyFence( device, inFlightFences[ i ], nullptr );
			inFlightFences[ i ] = VK_NULL_HANDLE;
		}
	}

	if ( commandPool != VK_NULL_HANDLE ) {
		vkDestroyCommandPool( device, commandPool, nullptr );
		commandPool = VK_NULL_HANDLE;
	}

	if ( allocator != VK_NULL_HANDLE ) {
		vmaDestroyAllocator( allocator );
		allocator = VK_NULL_HANDLE;
	}

	if ( device != VK_NULL_HANDLE ) {
		vkDestroyDevice( device, nullptr );
		device = VK_NULL_HANDLE;

		// These are cleaned up when the device is destroyed
		graphicsQueue = VK_NULL_HANDLE;
		presentQueue = VK_NULL_HANDLE;
	}

#if VK_DEBUG
	if ( debugCallback != VK_NULL_HANDLE ) {
		DestroyDebugUtilsMessengerEXT( nullptr );
		debugCallback = VK_NULL_HANDLE;
	}
#endif // VK_DEBUG

	// Destroy our window surface
	if ( surface != VK_NULL_HANDLE ) {
		vkDestroySurfaceKHR( instance, surface, nullptr );
		surface = VK_NULL_HANDLE;
	}

	// Destroy our instance
	if ( instance != VK_NULL_HANDLE ) {
		vkDestroyInstance( instance, nullptr );
		instance = VK_NULL_HANDLE;
		physicalDevice = VK_NULL_HANDLE; // This is cleaned up when the instance is destroyed
	}

	RequiredExtensions.clear();

	EngineSystem::unconfigure( engine );
}

void VulkanSystem::WaitIdle()
{
	if ( device != VK_NULL_HANDLE ) {
		vkDeviceWaitIdle( device );
	}
}

void VulkanSystem::NotifyWindowResized( uint32_t width, uint32_t height )
{
	vkDeviceWaitIdle( device );
	DestroySwapChain();

	CreateSwapChain( width, height );
	CreateImageViews();
	CreateRenderPass();
	CreateDepthResources();
	CreateFramebuffers();

	for ( auto vulkanInterface : vulkanInterfaces ) {
		vulkanInterface->onSwapChainResize();
	}
}

void VulkanSystem::RegisterInterface( VulkanInterface *vulkanInterface )
{
	if ( vulkanInterface == nullptr ) {
		return;
	}

	if ( std::find( vulkanInterfaces.begin(), vulkanInterfaces.end(), vulkanInterface ) != std::end( vulkanInterfaces ) ) {
		return;
	}

	vulkanInterfaces.push_back( vulkanInterface );
}

void VulkanSystem::UnregisterInterface( VulkanInterface *vulkanInterface )
{
	if ( vulkanInterface == nullptr ) {
		return;
	}

	if ( auto result = std::find( vulkanInterfaces.begin(), vulkanInterfaces.end(), vulkanInterface ); result != std::end( vulkanInterfaces ) ) {
		vulkanInterfaces.erase( result );
	}
}

void VulkanSystem::CreateShaderModule( const VkShaderModuleCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule )
{
	if ( vkCreateShaderModule( device, pCreateInfo, pAllocator, pShaderModule ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to create shader module" );
}

void VulkanSystem::DestroyShaderModule( VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator )
{
	vkDestroyShaderModule( device, shaderModule, pAllocator );
}

void VulkanSystem::CreateGraphicsPipelines( VkPipelineCache pipelineCache, uint32_t createInfoCount, const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines )
{
	if ( vkCreateGraphicsPipelines( device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to create graphics pipelines" );
}

void VulkanSystem::DestroyPipeline( VkPipeline pipeline, const VkAllocationCallbacks *pAllocator )
{
	vkDestroyPipeline( device, pipeline, pAllocator );
}

void VulkanSystem::CreatePipelineLayout( const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout )
{
	if ( vkCreatePipelineLayout( device, pCreateInfo, pAllocator, pPipelineLayout ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to create pipeline layout" );
}

void VulkanSystem::DestroyPipelineLayout( VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator )
{
	vkDestroyPipelineLayout( device, pipelineLayout, nullptr );
}

void VulkanSystem::CreateSampler( const VkSamplerCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkSampler *pSampler )
{
	if ( vkCreateSampler( device, pCreateInfo, pAllocator, pSampler ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to create sampler" );
}

void VulkanSystem::DestroySampler( VkSampler sampler, const VkAllocationCallbacks *pAllocator )
{
	vkDestroySampler( device, sampler, pAllocator );
}

void VulkanSystem::CreateDescriptorSetLayout( const VkDescriptorSetLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout )
{
	if ( vkCreateDescriptorSetLayout( device, pCreateInfo, pAllocator, pSetLayout ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to create descriptor set layout" );
}

void VulkanSystem::DestroyDescriptorSetLayout( VkDescriptorSetLayout descriptorSetLayout, const VkAllocationCallbacks *pAllocator )
{
	vkDestroyDescriptorSetLayout( device, descriptorSetLayout, pAllocator );
}

void VulkanSystem::CreateDescriptorPool( const VkDescriptorPoolCreateInfo *poolInfo, const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool )
{
	if ( vkCreateDescriptorPool( device, poolInfo, pAllocator, pDescriptorPool ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to create descriptor pool" );
}

void VulkanSystem::DestroyDescriptorPool( VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator )
{
	vkDestroyDescriptorPool( device, descriptorPool, pAllocator );
}

void VulkanSystem::AllocateDescriptorSets( const VkDescriptorSetAllocateInfo *pAllocateInfo, VkDescriptorSet *pDescriptorSets )
{
	if ( vkAllocateDescriptorSets( device, pAllocateInfo, pDescriptorSets ) != VK_SUCCESS )
		engine->Error( "[Vulkan]Failed to allocate descriptor sets" );
}

VkCommandBuffer VulkanSystem::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	if ( vkAllocateCommandBuffers( device, &allocInfo, &commandBuffer ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to allocate single use command buffer" );
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer( commandBuffer, &beginInfo );

	return commandBuffer;
}

void VulkanSystem::EndSingleTimeCommands( VkCommandBuffer &commandBuffer )
{
	vkEndCommandBuffer( commandBuffer );

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit( graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE );
	vkQueueWaitIdle( graphicsQueue );

	vkFreeCommandBuffers( device, commandPool, 1, &commandBuffer );
}

uint32_t VulkanSystem::FindMemoryType( uint32_t typeFilter, VkMemoryPropertyFlags properties )
{
	VkPhysicalDeviceMemoryProperties memProperties = {};
	vkGetPhysicalDeviceMemoryProperties( physicalDevice, &memProperties );

	for ( uint32_t i = 0; i < memProperties.memoryTypeCount; ++i )
	{
		if ( typeFilter & ( 1 << i ) && ( memProperties.memoryTypes[ i ].propertyFlags & properties ) == properties ) {
			return i;
		}
	}

	engine->Error( "[Vulkan]Failed to find suitable memory type" );

	// Should never get here
	return 0U;
}

void VulkanSystem::CopyBuffer( const VkBuffer &srcBuffer, VkBuffer &dstBuffer, VkDeviceSize size )
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		VkBufferCopy copyRegion = {};
		copyRegion.srcOffset = 0; // Optional
		copyRegion.dstOffset = 0; // Optional
		copyRegion.size = size;
		vkCmdCopyBuffer( commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion );
	EndSingleTimeCommands( commandBuffer );
}

void VulkanSystem::VmaMapMemory( VmaAllocation allocation, void **ppData )
{
	if ( vmaMapMemory( allocator, allocation, ppData ) != VK_SUCCESS )
		engine->Error( "[VulkanSystem]Failed to map memory" );
}

void VulkanSystem::VmaUnmapMemory( VmaAllocation allocation )
{
	vmaUnmapMemory( allocator, allocation );
}

void VulkanSystem::VmaCreateBuffer( VkDeviceSize size, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage, VkBuffer &buffer, VmaAllocation &allocation )
{
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = bufferUsage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryUsage;

	if ( vmaCreateBuffer( allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create buffer" );
	}
}

void VulkanSystem::VmaDestroyBuffer( VkBuffer buffer, VmaAllocation allocation )
{
	vmaDestroyBuffer( allocator, buffer, allocation );
}

VkImageView VulkanSystem::CreateImageView2D( VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels )
{
	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = mipLevels;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	VkImageView imageView = VK_NULL_HANDLE;
	if ( vkCreateImageView( device, &viewInfo, nullptr, &imageView ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create image view" );
	}

	return imageView;
}

void VulkanSystem::VmaCreateImage2D( uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage, VkImage &image, VmaAllocation &allocation )
{
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = mipLevels;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = imageUsage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.usage = memoryUsage;

	if ( vmaCreateImage( allocator, &imageInfo, &allocInfo, &image, &allocation, nullptr ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create image" );
	}
}

void VulkanSystem::CopyBufferToImage( VkBuffer buffer, VkImage image, uint32_t width, uint32_t height )
{
	std::vector< VkBufferImageCopy > bufferCopyRegions;

	VkBufferImageCopy region = {};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };
	bufferCopyRegions.push_back( region );

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		vkCmdCopyBufferToImage( commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, static_cast< uint32_t >( bufferCopyRegions.size() ), bufferCopyRegions.data() );
	EndSingleTimeCommands( commandBuffer );
}

void VulkanSystem::TransitionImageLayout( VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels )
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
		VkImageMemoryBarrier barrier = {};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
			
		if ( newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ) {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			auto hasStencilComponent = []( VkFormat format ) { return ( format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT ); };
				
			if ( hasStencilComponent( format ) ) {
				barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
			}
		}
		else {
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		}

		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = 0; // TODO
		barrier.dstAccessMask = 0; // TODO

		VkPipelineStageFlags sourceStage, destinationStage;

		if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL ) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}
		else if ( oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL ) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		else if ( oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ) {
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		else {
			engine->Error( "[Vulkan]Unsupported layout transition" );
		}

		vkCmdPipelineBarrier(
			commandBuffer,
			sourceStage, destinationStage,
			0,
			0, nullptr, 
			0, nullptr, 
			1, &barrier );

	EndSingleTimeCommands( commandBuffer );
}

void VulkanSystem::GenerateMipMaps( VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels )
{
	// Check if the image format supports linear blitting
	VkFormatProperties formatProperties = {};
	vkGetPhysicalDeviceFormatProperties( physicalDevice, imageFormat, &formatProperties );

	if ( !( formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ) ) {
		engine->Error( "[Vulkan]Failed to find linear blit tiling feature" );
	}

	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.image = image;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.subresourceRange.levelCount = 1;

	int32_t mipWidth = texWidth;
	int32_t mipHeight = texHeight;

	for ( uint32_t i = 1; i < mipLevels; ++i )
	{
		barrier.subresourceRange.baseMipLevel = i - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		VkImageBlit blit = {};
		blit.srcOffsets[ 0 ] = { 0, 0, 0 };
		blit.srcOffsets[ 1 ] = { mipWidth, mipHeight, 1 };
		blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.srcSubresource.mipLevel = i - 1;
		blit.srcSubresource.baseArrayLayer = 0;
		blit.srcSubresource.layerCount = 1;
		blit.dstOffsets[ 0 ] = { 0, 0, 0 };
		blit.dstOffsets[ 1 ] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
		blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		blit.dstSubresource.mipLevel = i;
		blit.dstSubresource.baseArrayLayer = 0;
		blit.dstSubresource.layerCount = 1;

		vkCmdBlitImage(
			commandBuffer,
			image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &blit,
			VK_FILTER_LINEAR
		);

		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		if ( mipWidth > 1 ) {
			mipWidth /= 2;
		}

		if ( mipHeight > 1 ) { 
			mipHeight /= 2;
		}
	}

	barrier.subresourceRange.baseMipLevel = mipLevels - 1;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	EndSingleTimeCommands( commandBuffer );
}