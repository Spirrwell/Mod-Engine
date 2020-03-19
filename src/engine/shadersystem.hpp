#ifndef SHADERSYSTEM_HPP
#define SHADERSYSTEM_HPP

#include <mutex>
#include <unordered_map>
#include <string_view>

#include "engine/ishadersystem.hpp"
#include "enginesystem.hpp"
#include "filesystem.hpp"
#include "vulkansystem.hpp"
#include "memory.hpp"
#include "shader.hpp"
#include "resourcepool.hpp"

class TextureSystem;

class ShaderSystem : public IShaderSystem, public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	void LoadShaders();

	IShader *FindShader( const std::string &shaderName ) const override;

private:
	IShader *FindShader_Internal( const std::string &shaderName ) const;

	template< typename T >
	void CreateShader( const std::string &shaderName, ResourcePool *resourcePool );

	FileSystem *fileSystem = nullptr;
	VulkanSystem *vulkanSystem = nullptr;

	mutable std::mutex shadersMutex;
};

#endif // SHADERSYSTEM_HPP