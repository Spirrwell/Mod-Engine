#ifndef RENDERSYSTEM_HPP
#define RENDERSYSTEM_HPP

#include "engine/irendersystem.hpp"
#include "enginesystem.hpp"
#include "glm/glm.hpp"
#include "memory.hpp"
#include "vulkansystem.hpp"
#include "mesh.hpp"
#include "renderlist.hpp"

#include <vector>

class MaterialSystem;
class ShaderSystem;

class RenderSystem : public IRenderSystem, public EngineSystem
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

	void QueueRender( const RenderInfo &renderInfo ) { activeRenderList[ imageIndex ].insert( renderInfo ); }

	void BeginFrame();
	void EndFrame();
	void DrawScene();

	void UpdateUBOs();
	void RecordCommandBuffer();

private:
	void ClearRenderLists();

	VulkanSystem *vulkanSystem = nullptr;
	MaterialSystem *materialSystem = nullptr;
	ShaderSystem *shaderSystem = nullptr;

	RenderView renderView = {};

	std::vector< RenderList > activeRenderList;
	std::vector< RenderList > lastRenderList;

	std::vector< VkCommandBuffer > commandBuffers;

	uint32_t imageIndex = 0;
	uint32_t currentFrame = 0;

	bool imageAcquireNeeded = false;
	bool isReadyToDraw = false;
	bool isMinimized = false;
};

#endif // RENDERSYSTEM_HPP