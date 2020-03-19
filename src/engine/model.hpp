#ifndef MODEL_HPP
#define MODEL_HPP

#include "engine/imodel.hpp"
#include "memory.hpp"
#include "mesh.hpp"

class Model : public IModel
{
public:
	static Model *ToModel( IModel *model ) { return static_cast< Model* >( model ); }

	std::vector< unique_ptr< Mesh > > meshes;
};

#endif // MODEL_HPP