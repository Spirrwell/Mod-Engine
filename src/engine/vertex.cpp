#include "vertex.hpp"
#include "log.hpp"

VertexLayout::VertexLayout( const std::multiset< Vertex::Component > &vertexComponents ) :
	vertexComponents( vertexComponents )
{
	for ( const auto &comp : vertexComponents )
		stride += (uint32_t)Vertex::GetComponentSize( comp );
}

size_t VertexLayout::GetOffset( Vertex::Component vertexComponent, size_t componentIndex ) const
{
	if ( componentIndex >= vertexComponents.count( vertexComponent ) )
		return InvalidOffset();

	auto compIt = vertexComponents.find( vertexComponent );
	std::advance( compIt, componentIndex );

	size_t offset = 0;
	for ( auto it = vertexComponents.cbegin(); it != compIt; ++it )
		offset += Vertex::GetComponentSize( *it );

	return offset;
}

VkVertexInputBindingDescription VertexLayout::ToInputBindingDescription( uint32_t binding ) const
{
	VkVertexInputBindingDescription description = {};
	description.binding = binding;
	description.stride = stride;
	description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	return description;
}

std::vector< VkVertexInputAttributeDescription > VertexLayout::ToInputAttributeDescriptions( uint32_t binding ) const
{
	std::vector< VkVertexInputAttributeDescription > descriptions;
	descriptions.resize( vertexComponents.size() );

	auto computeOffset = []( const std::multiset< Vertex::Component > &vertexComponents, size_t index ) -> uint32_t
	{
		uint32_t offset = 0;

		if ( index < vertexComponents.size() )
		{
			for ( size_t i = 0; i < index; ++i )
			{
				auto compIt = vertexComponents.cbegin();
				std::advance( compIt, i );

				offset += (uint32_t)Vertex::GetComponentSize( *compIt );
			}
		}

		return offset;
	};

	for ( size_t i = 0; i < descriptions.size(); ++i )
	{
		auto &desc = descriptions[ i ];
		desc.binding = binding;
		desc.location = (uint32_t)i;
		desc.offset = computeOffset( vertexComponents, i );

		auto compIt = vertexComponents.cbegin();
		std::advance( compIt, i );

		switch ( (*compIt) )
		{
			case Vertex::Component::Position:
			case Vertex::Component::Normal:
			case Vertex::Component::Tangent:
			case Vertex::Component::BiTangent:
				desc.format = VK_FORMAT_R32G32B32_SFLOAT;
				break;
			case Vertex::Component::Color:
				desc.format = VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			case Vertex::Component::UV:
				desc.format = VK_FORMAT_R32G32_SFLOAT;
				break;
		}
	}

	return descriptions;
}

size_t Vertex::GetComponentSize( Vertex::Component component )
{
	static size_t vertexComponentSizes[ (size_t)Vertex::Component::Count ] = {
		sizeof( glm::vec3 ), // Position
		sizeof( glm::vec3 ), // Normal
		sizeof( glm::vec4 ), // Color
		sizeof( glm::vec2 ), // UV
		sizeof( glm::vec3 ), // Tangent
		sizeof( glm::vec3 )  // BiTangent
	};

	return vertexComponentSizes[ (size_t)component ];
}

VertexArray::VertexArray( const VertexLayout &vertexLayout ) :
	vertexLayout( vertexLayout )
{
}

size_t VertexArray::CreateVertex()
{
	const size_t vertexIndex = vertexCount;
	Resize( vertexCount + 1 );

	return vertexIndex;
}

void VertexArray::Resize( size_t newVertexCount )
{
	vertexBuffer.resize( vertexLayout.GetStride() * newVertexCount );
	vertexCount = newVertexCount;
}

void VertexArray::SetPosition( size_t vertexIndex, const glm::vec3 &position, size_t positionIndex )
{
	Set( vertexIndex, vertexLayout.GetOffset( Vertex::Component::Position, positionIndex ), (std::byte*)&position, sizeof( position ) );
}

void VertexArray::SetNormal( size_t vertexIndex, const glm::vec3 &normal, size_t normalIndex )
{
	Set( vertexIndex, vertexLayout.GetOffset( Vertex::Component::Normal, normalIndex ), (std::byte*)&normal, sizeof( normal ) );
}

void VertexArray::SetColor( size_t vertexIndex, const glm::vec4 &color, size_t colorIndex )
{
	Set( vertexIndex, vertexLayout.GetOffset( Vertex::Component::Color, colorIndex ), (std::byte*)&color, sizeof( color ) );
}

void VertexArray::SetUV( size_t vertexIndex, const glm::vec2 &uv, size_t uvIndex )
{
	Set( vertexIndex, vertexLayout.GetOffset( Vertex::Component::UV, uvIndex ), (std::byte*)&uv, sizeof( uv ) );
}

void VertexArray::SetTangent( size_t vertexIndex, const glm::vec3 &tangent, size_t tangentIndex )
{
	Set( vertexIndex, vertexLayout.GetOffset( Vertex::Component::Tangent, tangentIndex ), (std::byte*)&tangent, sizeof( tangent ) );
}

void VertexArray::SetBiTangent( size_t vertexIndex, const glm::vec3 &bitangent, size_t bitangentIndex )
{
	Set( vertexIndex, vertexLayout.GetOffset( Vertex::Component::BiTangent, bitangentIndex ), (std::byte*)&bitangent, sizeof( bitangent ) );
}

void VertexArray::Set( size_t vertexIndex, size_t componentOffset, const std::byte *data, size_t dataSize )
{
	if ( IsValidVertex( vertexIndex ) )
	{
		if ( componentOffset != VertexLayout::InvalidOffset() )
		{
			const size_t vertexOffset = GetVertexOffset( vertexIndex );
			const size_t dataStart = vertexOffset + componentOffset;
			const size_t dataEnd = dataStart + dataSize;

			if ( dataStart < vertexBuffer.size() && dataEnd <= vertexBuffer.size() )
				std::memcpy( &vertexBuffer[ dataStart ], data, dataSize );
			else
			{
				Log::PrintlnWarn( "{} out of range!", __FUNCTION__ );
			}
		}
	}
	else
	{
		Log::PrintlnWarn( "Invalid vertex {}", vertexIndex );
	}
}

size_t VertexArray::GetVertexOffset( size_t vertexIndex ) const
{
	return ( (size_t)vertexLayout.GetStride() * vertexIndex );
}

bool VertexArray::IsValidVertex( size_t vertexIndex ) const
{
	return ( vertexIndex < vertexCount );
}