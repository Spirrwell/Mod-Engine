#include "resourcepool.hpp"

ResourcePool::ResourcePool( IResourcePool *parent )
{
	if ( parent )
	{
		ResourcePool *other = ToResourcePool( parent );

		textures = other->textures;
		shaders = other->shaders;
		materials = other->materials;
		models = other->models;
	}
}