#ifndef MODEL_HPP
#define MODEL_HPP

#include "engine/imodel.hpp"
#include "memory.hpp"
#include "mesh.hpp"

class MeshSystem;

class Model : public IModel
{
public:
	Model( MeshSystem *meshSystem );
	~Model();
	
	static Model *ToModel( IModel *model ) { return static_cast< Model* >( model ); }

	std::vector< Mesh* > meshes;
	MeshSystem *meshSystem = nullptr;
};

#endif // MODEL_HPP