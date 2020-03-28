#ifndef MESH_HPP
#define MESH_HPP

#include "engine/imesh.hpp"
#include "vulkansystem.hpp"
#include "memory.hpp"
#include "vertex.hpp"
#include "ubo.hpp"

#include <vector>

class Material;

class Mesh : public IMesh, public VulkanInterface
{
public:
	using VulkanInterface::VulkanInterface;
	~Mesh();

	static Mesh *ToMesh( IMesh *mesh ) { return static_cast< Mesh* >( mesh ); }

	void Init( VulkanSystem *vulkanSystem, shared_ptr< VertexArray > vertices, shared_ptr< std::vector< uint32_t > > indices, Material *material );

	void onSwapChainResize() override;
	void destroySwapChain();

	const VkBuffer GetVertexBuffer() const { return VertexBuffer; }
	const VkBuffer GetIndexBuffer() const { return IndexBuffer; }

	uint32_t GetVertexCount() const { return vertexCount; }
	uint32_t GetIndexCount() const { return indexCount; }

	Material *GetMaterial() const { return material; }

	void CreateVertexBuffer();
	void CreateIndexBuffer();

	shared_ptr< VertexArray > vertices;
	shared_ptr< std::vector< uint32_t > > indices;

	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

	Material *material = nullptr;

	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector< VkDescriptorSet > descriptorSets;
	std::vector< unique_ptr< UBO > > ubos;

	VkBuffer VertexBuffer = VK_NULL_HANDLE;
	VmaAllocation VertexBufferAllocation = VK_NULL_HANDLE;

	VkBuffer IndexBuffer = VK_NULL_HANDLE;
	VmaAllocation IndexBufferAllocation = VK_NULL_HANDLE;
};

#endif // MESH_HPP