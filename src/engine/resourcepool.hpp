#ifndef RESOURCEPOOL_HPP
#define RESOURCEPOOL_HPP

#include "engine/iresourcepool.hpp"
#include "resource.hpp"

#include "texture.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "model.hpp"

#include <type_traits>

class ResourcePool : public IResourcePool
{
public:
	ResourcePool( IResourcePool *parent );

	static inline ResourcePool *ToResourcePool( IResourcePool *resourcePool ) { return static_cast< ResourcePool* >( resourcePool ); }

	// At some point we could move to RTTI allocation of resource lists for syntactic sugar
	// The overhead probably wouldn't be THAT bad
	ResourceList< Texture > textures;
	ResourceList< Shader > shaders;
	ResourceList< Material > materials;
	ResourceList< Model > models;

	template < typename T, typename Base = T, typename ... Args >
	static shared_ptr< Resource< Base > > createResource( const ResourceInfo &resourceInfo, Args &&... args )
	{
		shared_ptr< Resource< Base > > resource = make_shared< Resource< Base > >();
		resource->resource = make_shared< T >( args... );
		resource->resourceInfo = resourceInfo;

		return resource;
	}

	template < typename T >
	T *findResource( const std::string &identifier ) const;

private:
	template < typename T >
	T *findResource( const std::string &identifier, const ResourceList< T > &resourceList ) const;
};

template < typename T >
T *ResourcePool::findResource( const std::string &identifier ) const
{
	if constexpr ( std::is_same_v< T, Texture > )
	{
		return findResource( identifier, textures );
	}

	if constexpr ( std::is_same_v< T, Shader > )
	{
		return findResource( identifier, shaders );
	}

	if constexpr ( std::is_same_v< T, Material > )
	{
		return findResource( identifier, materials );
	}

	if constexpr ( std::is_same_v< T, Model > )
	{
		return findResource( identifier, models );
	}

	return nullptr;
}

template < typename T >
T *ResourcePool::findResource( const std::string &identifier, const ResourceList< T > &resourceList ) const
{
	for ( const auto &resource : resourceList )
	{
		if ( resource->resourceInfo.identifier == identifier )
			return resource->resource.get();
	}

	return nullptr;
}

#endif // RESOURCEPOOL_HPP