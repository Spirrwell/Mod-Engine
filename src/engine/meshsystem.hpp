#ifndef MESHSYSTEM_HPP
#define MESHSYSTEM_HPP

#include "enginesystem.hpp"
#include "memory.hpp"
#include "mesh.hpp"
#include "vulkansystem.hpp"
#include "rendersystem.hpp"

#include <vector>
#include <limits>

class MeshSystem : public EngineSystem
{
public:
	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	Mesh *CreateMesh();

	void DestroyMesh( Mesh *mesh );

private:

	std::vector< unique_ptr< Mesh > > meshes;
	VulkanSystem *vulkanSystem = nullptr;
	RenderSystem *renderSystem = nullptr;
};

#endif // MESHSYSTEM_HPP