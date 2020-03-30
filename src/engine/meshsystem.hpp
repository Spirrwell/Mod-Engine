#ifndef MESHSYSTEM_HPP
#define MESHSYSTEM_HPP

#include "enginesystem.hpp"
#include "memory.hpp"
#include "mesh.hpp"
#include "vulkansystem.hpp"

#include <set>
#include <vector>
#include <limits>
#include <mutex>

class MeshSystem : public EngineSystem
{
public:
	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	Mesh *CreateMesh();

	// Adds mesh to destroy list
	void DestroyMesh( Mesh *mesh );

	// Destroys meshes in destroy list and re-sets mesh indices
	void DestroyDeadMeshes();

private:

	std::vector< unique_ptr< Mesh > > meshes;
	std::set< size_t > destroyList;

	std::mutex meshMutex;

	VulkanSystem *vulkanSystem = nullptr;
};

#endif // MESHSYSTEM_HPP