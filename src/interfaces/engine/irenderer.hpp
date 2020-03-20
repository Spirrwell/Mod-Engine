#ifndef IRENDERER_HPP
#define IRENDERER_HPP

#include "glm/glm.hpp"
#include "renderview.hpp"
#include "engine/imesh.hpp"
#include "engine/imodel.hpp"

class IRenderSystem
{
public:
	virtual ~IRenderSystem() = default;

	virtual RenderView &GetRenderView() = 0;

	// Draws specified mesh with a 'model' matrix transformation
	virtual void DrawMesh( IMesh *mesh, const glm::mat4 &modelMat ) = 0;

	// Draws specified model with a 'model' matrix transformation, calls DrawMesh for all meshes in model
	virtual void DrawModel( IModel *model, const glm::mat4 &modelMat ) = 0;
};

#endif // IRENDERER_HPP