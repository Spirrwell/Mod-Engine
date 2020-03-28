#ifndef VERTEX_HPP
#define VERTEX_HPP

#include "glm/glm.hpp"
#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstddef>
#include <limits>
#include <vector>
#include <set>

struct Vertex
{
	enum class Component : uint32_t
	{
		Position,
		Normal,
		Color,
		UV,
		Tangent,
		BiTangent,
		Count
	};

	static size_t GetComponentSize( Vertex::Component component );
};

struct VertexLayout
{
	VertexLayout( const std::multiset< Vertex::Component > &vertexComponents );

	constexpr static inline size_t InvalidOffset() { return std::numeric_limits< size_t >::max(); }

	const std::multiset< Vertex::Component > &GetComponents() const { return vertexComponents; }

	// Determines the offset of a given vertex component
	size_t GetOffset( Vertex::Component vertexComponent, size_t componentIndex ) const;
	uint32_t GetStride() const { return stride; }

	VkVertexInputBindingDescription ToInputBindingDescription( uint32_t binding ) const;
	std::vector< VkVertexInputAttributeDescription > ToInputAttributeDescriptions( uint32_t binding ) const;

private:

	const std::multiset< Vertex::Component > vertexComponents;
	uint32_t stride = 0;
};

struct VertexArray
{
	VertexArray( const VertexLayout &vertexLayout );

	// Adds new vertex to vertex buffer, returns vertex index of newly created vertex
	size_t CreateVertex();

	// Resizes vertex buffer, if newVetexCount is less than current count, vertices after newVertexCount will be cleared
	void Resize( size_t newVertexCount );

	void SetPosition( size_t vertexIndex, const glm::vec3 &position, size_t positionIndex );
	void SetNormal( size_t vertexIndex, const glm::vec3 &normal, size_t normalIndex );
	void SetColor( size_t vertexIndex, const glm::vec4 &color, size_t colorIndex );
	void SetUV( size_t vertexIndex, const glm::vec2 &uv, size_t uvIndex );
	void SetTangent( size_t vertexIndex, const glm::vec3 &tangent, size_t tangentIndex );
	void SetBiTangent( size_t vertexIndex, const glm::vec3 &bitangent, size_t bitangentIndex );

	inline const std::byte *GetVertexBuffer() const { return vertexBuffer.data(); }
	inline size_t GetVertexBufferSize() const { return vertexBuffer.size(); }
	inline size_t GetVertexCount() const { return vertexCount; }

private:
	void Set( size_t vertexIndex, size_t componentOffset, const std::byte *data, size_t dataSize );

	size_t GetVertexOffset( size_t vertexIndex ) const;
	bool IsValidVertex( size_t vertexIndex ) const;

	const VertexLayout vertexLayout;
	std::vector< std::byte > vertexBuffer;

	size_t vertexCount = 0;
};

#endif // VERTEX_HPP