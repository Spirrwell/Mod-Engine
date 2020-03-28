#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#include <unordered_map>

#include "engine/ishader.hpp"
#include "filesystem.hpp"
#include "vulkansystem.hpp"
#include "memory.hpp"
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"
#include "glm/glm.hpp"
#include "ubo.hpp"
#include "log.hpp"
#include "mesh.hpp"
#include "vertex.hpp"

class RenderSystem;
class Material;

enum class ShaderType : size_t
{
	Vertex,
	Fragment,
	Max
};

struct MVP
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class Shader : public IShader, public VulkanInterface
{
	friend class RenderSystem;
	friend class ShaderSystem;

public:
	Shader( VulkanSystem *vulkanSystem, const std::string &shaderName );
	virtual ~Shader();

	static inline Shader *ToShader( IShader *shader ) { return static_cast< Shader* >( shader ); }

	inline virtual VertexLayout GetVertexLayout() const = 0;

	virtual VkDescriptorPool CreateDescriptorPool() const;
	virtual void InitMaterial( Material &material ) = 0;

	virtual void InitMesh( Mesh *mesh ) = 0;

	const std::string &GetShaderName() const { return shaderName; }
	const VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptorSetLayout; }

private:
	void onSwapChainResize() override final;
	void DestroySwapChainElements();

	template < ShaderType shaderType >
	void ReadShaderFile( FileSystem *fileSystem, std::vector< char > &buffer ) const;

	VkShaderModule CreateShaderModule( const std::vector< char > &code );

protected:
	VkPipelineLayout GetPipelineLayout() const { return pipelineLayout; }
	VkPipeline GetPipeline() const { return pipeline; }

	virtual void CreateDescriptorSetLayout() = 0;
	virtual void CreateGraphicsPipelineLayout() = 0;
	virtual void CreateGraphicsPipeline() = 0;

	virtual void Update( const uint32_t imageIndex, const MVP &mvp, Mesh *mesh ) = 0;

	void CreateShaderModules( FileSystem *fileSystem );

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkShaderModule shaderModules[ static_cast< size_t >( ShaderType::Max ) ] = { VK_NULL_HANDLE, VK_NULL_HANDLE };
	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline pipeline = VK_NULL_HANDLE;

private:
	const std::string shaderName;
};

#endif // SHADER_HPP