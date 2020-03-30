#include "model.hpp"
#include "meshsystem.hpp"

Model::Model( MeshSystem *meshSystem ) :
	meshSystem( meshSystem )
{
}

Model::~Model()
{
	for ( auto mesh : meshes )
		meshSystem->DestroyMesh( mesh );

	meshes.clear();
}