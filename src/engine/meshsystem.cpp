#include "meshsystem.hpp"
#include "engine.hpp"

void MeshSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );
	vulkanSystem = engine->GetVulkanSystem();
	renderSystem = engine->GetRenderSystem();
}

void MeshSystem::unconfigure( Engine *engine )
{
	renderSystem = nullptr;
	vulkanSystem = nullptr;
	EngineSystem::unconfigure( engine );
}

Mesh *MeshSystem::CreateMesh()
{
	meshes.push_back( make_unique< Mesh >( vulkanSystem ) );
	auto &mesh = meshes.back();

	mesh->meshIndex = meshes.size() - 1;
	return mesh.get();
}

void MeshSystem::DestroyMesh( Mesh *mesh )
{
	meshes.erase( meshes.begin() + mesh->meshIndex );

	for ( size_t i = 0; i < meshes.size(); ++i )
		meshes[ i ]->meshIndex = i;
}