#include "meshsystem.hpp"
#include "engine.hpp"

void MeshSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );
	vulkanSystem = engine->GetVulkanSystem();
}

void MeshSystem::unconfigure( Engine *engine )
{
	DestroyDeadMeshes();

	vulkanSystem = nullptr;
	EngineSystem::unconfigure( engine );
}

Mesh *MeshSystem::CreateMesh()
{
	std::lock_guard< std::mutex > lock( meshMutex );
	meshes.push_back( make_unique< Mesh >( vulkanSystem ) );
	auto &mesh = meshes.back();

	mesh->meshIndex = meshes.size() - 1;
	return mesh.get();
}

void MeshSystem::DestroyMesh( Mesh *mesh )
{
	destroyList.insert( mesh->meshIndex );
}

void MeshSystem::DestroyDeadMeshes()
{
	if ( destroyList.size() > 0 )
	{
		for ( auto it = destroyList.crbegin(); it != destroyList.crend(); ++it )
			meshes.erase( meshes.begin() + (*it) );

		destroyList.clear();

		for ( size_t i = 0; i < meshes.size(); ++i )
			meshes[ i ]->meshIndex = i;
	}
}