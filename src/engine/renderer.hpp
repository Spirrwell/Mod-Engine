#ifndef RENDERER_HPP
#define RENDERER_HPP

#include "engine/irenderer.hpp"
#include "enginesystem.hpp"
#include "glm/glm.hpp"
#include "memory.hpp"
#include "vulkansystem.hpp"
#include "mesh.hpp"

#include <vector>

class MaterialSystem;
class ShaderSystem;

struct RenderInfo
{
	Mesh *mesh = nullptr;
	glm::mat4 modelMat;
};

class Renderer : public IRenderer, public EngineSystem
{
public:

	void configure( Engine *engine ) override;
	void unconfigure( Engine *engine ) override;

	void WaitIdle();

	RenderView &GetRenderView() override { return renderView; }

	// Draws specified mesh with a 'model' matrix transformation
	void DrawMesh( IMesh *mesh, const glm::mat4 &modelMat ) override;

	// Draws specified model with a 'model' matrix transformation, calls DrawMesh for all meshes in model
	void DrawModel( IModel *model, const glm::mat4 &modelMat ) override;

	void NotifyWindowResized( uint32_t width, uint32_t height );
	void NotifyWindowMaximized();
	void NotifyWindowMinimized();

	VulkanSystem *GetVulkanSystem() const { return vulkanSystem; }
	MaterialSystem *GetMaterialSystem() const { return materialSystem; }
	ShaderSystem *GetShaderSystem() const { return shaderSystem; }

	void QueueRender( RenderInfo renderInfo ) { renderInfos.push_back( renderInfo ); }

	void BeginFrame();
	void EndFrame();
	void DrawScene();

	void RecordCommandBuffer();

private:
	VulkanSystem *vulkanSystem = nullptr;
	MaterialSystem *materialSystem = nullptr;
	ShaderSystem *shaderSystem = nullptr;

	RenderView renderView = {};

	std::vector< RenderInfo > renderInfos;
	std::vector< VkCommandBuffer > commandBuffers;

	uint32_t imageIndex = 0;
	uint32_t currentFrame = 0;

	bool isReadyToDraw = false;
	bool isMinimized = false;
};

#endif // RENDERER_HPP