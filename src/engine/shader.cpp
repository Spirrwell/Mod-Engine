#include "shader.hpp"
#include "engine.hpp"
#include "vertex.hpp"
#include "material.hpp"
#include "log.hpp"
#include "shadersystem.hpp"
#include "renderer.hpp"

#include <filesystem>
#include <fstream>
#include <array>
#include <string_view>
#include <algorithm>

Shader::Shader( VulkanSystem *vulkanSystem, const std::string &shaderName ) :
	VulkanInterface( vulkanSystem ),
	shaderName( shaderName )
{
}

Shader::~Shader()
{
	vulkanSystem->WaitIdle();

	for ( auto &shaderModule : shaderModules )
	{
		if ( shaderModule != VK_NULL_HANDLE ) {
			vulkanSystem->DestroyShaderModule( shaderModule, nullptr );
			shaderModule = VK_NULL_HANDLE;
		}
	}

	DestroySwapChainElements();

	if ( pipelineLayout != VK_NULL_HANDLE ) {
		vulkanSystem->DestroyPipelineLayout( pipelineLayout, nullptr );
		pipelineLayout = VK_NULL_HANDLE;
	}

	if ( descriptorSetLayout != VK_NULL_HANDLE ) {
		vulkanSystem->DestroyDescriptorSetLayout( descriptorSetLayout, nullptr );
		descriptorSetLayout = VK_NULL_HANDLE;
	}
}

VkDescriptorPool Shader::CreateDescriptorPool() const
{
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	std::array< VkDescriptorPoolSize, 2 > poolSizes;
	poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[ 0 ].descriptorCount = vulkanSystem->numSwapChainImages;
	poolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[ 1 ].descriptorCount = vulkanSystem->numSwapChainImages;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = vulkanSystem->numSwapChainImages;

	vulkanSystem->CreateDescriptorPool( &poolInfo, nullptr, &descriptorPool );

	return descriptorPool;
}

void Shader::onSwapChainResize()
{
	DestroySwapChainElements();
	CreateGraphicsPipeline();
}

void Shader::DestroySwapChainElements()
{
	// Swap Chain
	if ( pipeline != VK_NULL_HANDLE ) {
		vulkanSystem->DestroyPipeline( pipeline, nullptr );
		pipeline = VK_NULL_HANDLE;
	}
}

template< ShaderType shaderType >
void Shader::ReadShaderFile( FileSystem *fileSystem, std::vector< char > &buffer ) const
{
	constexpr const std::string_view shaderExtensions[ static_cast< size_t >( ShaderType::Max ) ] = {
		".vert.spv",
		".frag.spv"
	};

	const std::string_view &extension = shaderExtensions[ static_cast< size_t >( shaderType ) ];
	const std::filesystem::path fileName = fmt::format( "shaders/{}{}", shaderName, extension );

	Log::Println( "Loading shader file {}", fileName.string() );

	if ( !fileSystem->ReadToBuffer( fileName, "GAME", buffer ) ) {
		Log::PrintlnWarn( fmt::format( "Failed to load shader file for shader: {}", shaderName ) );
	}
}

VkShaderModule Shader::CreateShaderModule( const std::vector< char > &code )
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast< const uint32_t* >( code.data() );

	VkShaderModule shaderModule = VK_NULL_HANDLE;
	vulkanSystem->CreateShaderModule( &createInfo, nullptr, &shaderModule );

	return shaderModule;
}

void Shader::CreateShaderModules( FileSystem *fileSystem )
{
	std::vector< char > vertexCode;
	std::vector< char > fragmentCode;

	ReadShaderFile< ShaderType::Vertex >( fileSystem, vertexCode );
	ReadShaderFile< ShaderType::Fragment >( fileSystem, fragmentCode );

	shaderModules[ static_cast< size_t >( ShaderType::Vertex ) ] = CreateShaderModule( vertexCode );
	shaderModules[ static_cast< size_t >( ShaderType::Fragment ) ] = CreateShaderModule( fragmentCode );
}