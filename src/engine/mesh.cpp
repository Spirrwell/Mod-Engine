#include "mesh.hpp"
#include "vk_mem_alloc.h"
#include "renderer.hpp"
#include "material.hpp"

Mesh::~Mesh()
{
	vulkanSystem->WaitIdle();

	destroySwapChain();

	if ( IndexBuffer != VK_NULL_HANDLE ) {
		vulkanSystem->VmaDestroyBuffer( IndexBuffer, IndexBufferAllocation );
		IndexBuffer = VK_NULL_HANDLE;
		IndexBufferAllocation = VK_NULL_HANDLE;
	}	

	if ( VertexBuffer != VK_NULL_HANDLE ) {
		vulkanSystem->VmaDestroyBuffer( VertexBuffer, VertexBufferAllocation );
		VertexBuffer = VK_NULL_HANDLE;
		VertexBufferAllocation = VK_NULL_HANDLE;
	}
}

void Mesh::Init( VulkanSystem *vulkanSystem, shared_ptr< std::vector< Vertex > > vertices, shared_ptr< std::vector< uint32_t > > indices, Material *material )
{
	this->vulkanSystem = vulkanSystem;
	this->vertices = vertices;
	this->indices = indices;
	this->material = material;

	CreateVertexBuffer();
	CreateIndexBuffer();

	if ( material && material->GetShader() )
		material->GetShader()->InitMesh( this );
}

void Mesh::onSwapChainResize()
{
	destroySwapChain();

	if ( material && material->GetShader() )
		material->GetShader()->InitMesh( this );
}

void Mesh::destroySwapChain()
{
	ubos.clear();

	if ( descriptorPool != VK_NULL_HANDLE )
	{
		vulkanSystem->DestroyDescriptorPool( descriptorPool, nullptr );
		descriptorPool = VK_NULL_HANDLE;
	}

	descriptorSets.clear();
}

void Mesh::CreateVertexBuffer()
{
	if ( vertices->size() == 0 ) {
		return;
	}

	const VkDeviceSize bufferSize = sizeof( Vertex ) * vertices->size();
	vertexCount = static_cast< uint32_t >( vertices->size() );

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

	vulkanSystem->VmaCreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferAllocation );

	void *pData = nullptr;

	vulkanSystem->VmaMapMemory( stagingBufferAllocation, &pData );
		std::memcpy( pData, vertices->data(), static_cast< size_t >( bufferSize ) );
	vulkanSystem->VmaUnmapMemory( stagingBufferAllocation );

	vulkanSystem->VmaCreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, VertexBuffer, VertexBufferAllocation );
	vulkanSystem->CopyBuffer( stagingBuffer, VertexBuffer, bufferSize );

	vulkanSystem->VmaDestroyBuffer( stagingBuffer, stagingBufferAllocation );
}

void Mesh::CreateIndexBuffer()
{
	if ( indices->size() == 0 ) {
		return;
	}

	VkDeviceSize bufferSize = sizeof( uint32_t ) * indices->size();
	indexCount = static_cast< uint32_t >( indices->size() );

	VkBuffer stagingBuffer = VK_NULL_HANDLE;
	VmaAllocation stagingBufferAllocation = VK_NULL_HANDLE;

	vulkanSystem->VmaCreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferAllocation );

	void *pData = nullptr;

	vulkanSystem->VmaMapMemory( stagingBufferAllocation, &pData );
		std::memcpy( pData, indices->data(), static_cast< size_t >( bufferSize ) );
	vulkanSystem->VmaUnmapMemory( stagingBufferAllocation );

	vulkanSystem->VmaCreateBuffer( bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, IndexBuffer, IndexBufferAllocation );
	vulkanSystem->CopyBuffer( stagingBuffer, IndexBuffer, bufferSize );

	vulkanSystem->VmaDestroyBuffer( stagingBuffer, stagingBufferAllocation );
}