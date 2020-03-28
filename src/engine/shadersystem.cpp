#include "shadersystem.hpp"
#include "engine.hpp"
#include "shader.hpp"
#include "basicshader.hpp"
#include "wireshader.hpp"
#include "texturesystem.hpp"
#include "nlohmann/json.hpp"

#include "basicshader.hpp"
#include "wireshader.hpp"

void ShaderSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );
	fileSystem = engine->GetFileSystem();
	vulkanSystem = engine->GetVulkanSystem();
}

void ShaderSystem::unconfigure( Engine *engine )
{
	fileSystem = nullptr;
	vulkanSystem = nullptr;
	EngineSystem::unconfigure( engine );
}

void ShaderSystem::LoadShaders()
{
	ResourcePool *resourcePool = ResourcePool::ToResourcePool( engine->GetGlobalResourcePool() );

	if ( !resourcePool )
		engine->Error( "[ShaderSystem]Failed to acquire global resource pool!" );

	CreateShader< BasicShader >( "BasicShader", resourcePool );
	CreateShader< WireShader >( "WireShader", resourcePool );
}

IShader *ShaderSystem::FindShader( const std::string &shaderName ) const
{
	if ( ResourcePool *resourcePool = ResourcePool::ToResourcePool( engine->GetGlobalResourcePool() ); resourcePool )
		return resourcePool->findResource< Shader >( shaderName );

	return nullptr;
}

IShader *ShaderSystem::FindShader_Internal( const std::string &shaderName ) const
{
	// We only need to lock guard our own lookups
	std::lock_guard< std::mutex > lock( shadersMutex );
	return FindShader( shaderName );
}

template< typename T >
void ShaderSystem::CreateShader( const std::string &shaderName, ResourcePool *resourcePool )
{
	auto resource = ResourcePool::createResource< T, Shader >( ResourceInfo{ shaderName }, vulkanSystem, shaderName );
	auto shader = resource->resource.get();

	shader->CreateDescriptorSetLayout();
	shader->CreateShaderModules( fileSystem );
	shader->CreateGraphicsPipelineLayout();
	shader->CreateGraphicsPipeline();

	resourcePool->shaders.push_back( resource );
}