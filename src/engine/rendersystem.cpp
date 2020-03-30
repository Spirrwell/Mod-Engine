#include "rendersystem.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "material.hpp"
#include "shadersystem.hpp"
#include "shader.hpp"

void RenderSystem::configure( Engine *engine )
{
	EngineSystem::configure( engine );

	vulkanSystem = engine->GetVulkanSystem();
	materialSystem = engine->GetMaterialSystem();
	shaderSystem = engine->GetShaderSystem();

	commandBuffers.resize( vulkanSystem->numSwapChainImages );

	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = vulkanSystem->commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = vulkanSystem->numSwapChainImages;

	if ( vkAllocateCommandBuffers( vulkanSystem->device, &allocInfo, commandBuffers.data() ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to allocate command buffers" );
	}

	if ( vkAcquireNextImageKHR( vulkanSystem->device, vulkanSystem->swapChainKHR, std::numeric_limits< uint64_t >::max(), vulkanSystem->imageAvailableSemaphores[ currentFrame ], VK_NULL_HANDLE, &imageIndex ) != VK_SUCCESS ) {
		Log::PrintlnWarn( "[Vulkan]Failed to acquire swap chain image" );
		imageAcquireNeeded = true;
	}

	activeRenderList.resize( vulkanSystem->numSwapChainImages );
	lastRenderList.resize( vulkanSystem->numSwapChainImages );

	const float aspect = ( float )vulkanSystem->swapChainExtent.width / ( float )vulkanSystem->swapChainExtent.height;
	renderView.viewMatrix = glm::mat4( 1.0f );
	renderView.projectionMatrix = glm::perspective( glm::radians( 70.0f ), aspect, 0.01f, 10000.0f );
}

void RenderSystem::unconfigure( Engine *engine )
{
	WaitIdle();

	if ( commandBuffers.size() > 0 ) {
		vkFreeCommandBuffers( vulkanSystem->device, vulkanSystem->commandPool, vulkanSystem->numSwapChainImages, commandBuffers.data() );
		commandBuffers.clear();
	}

	shaderSystem = nullptr;
	materialSystem = nullptr;
	vulkanSystem = nullptr;

	EngineSystem::unconfigure( engine );
}

void RenderSystem::WaitIdle()
{
	if ( vulkanSystem->device != VK_NULL_HANDLE ) {
		vkDeviceWaitIdle( vulkanSystem->device );
	}
}

void RenderSystem::DrawMesh( IMesh *mesh, const glm::mat4 &modelMat )
{
	QueueRender( RenderInfo{ Mesh::ToMesh( mesh ), modelMat } );
}

// Draws specified model with a 'model' matrix transformation, calls DrawMesh for all meshes in model
void RenderSystem::DrawModel( IModel *model, const glm::mat4 &modelMat )
{
	Model *realModel = Model::ToModel( model );
	for ( auto mesh : realModel->meshes )
		DrawMesh( mesh, modelMat );
}

void RenderSystem::NotifyWindowResized( uint32_t width, uint32_t height )
{
	vulkanSystem->NotifyWindowResized( width, height );
	//shaderSystem->NotifyWindowResized();
}

void RenderSystem::NotifyWindowMaximized()
{
	isMinimized = false;
}

void RenderSystem::NotifyWindowMinimized()
{
	isMinimized = true;
}

void RenderSystem::BeginFrame()
{
	if ( isMinimized ) {
		activeRenderList[ imageIndex ].clear();
		return;
	}

	if ( imageAcquireNeeded )
	{
		vkWaitForFences( vulkanSystem->device, 1, &vulkanSystem->inFlightFences[ currentFrame ], VK_TRUE, std::numeric_limits< uint64_t >::max() );

		if ( vkAcquireNextImageKHR( vulkanSystem->device, vulkanSystem->swapChainKHR, std::numeric_limits< uint64_t >::max(), vulkanSystem->imageAvailableSemaphores[ currentFrame ], VK_NULL_HANDLE, &imageIndex ) != VK_SUCCESS ) {
			Log::PrintlnWarn( "[Vulkan]Failed to acquire swap chain image" );
			ClearRenderLists();
			return;
		}

		imageAcquireNeeded = false;
	}

	isReadyToDraw = true;
}

void RenderSystem::EndFrame()
{
	isReadyToDraw = false;
}

void RenderSystem::DrawScene()
{
	if ( !isReadyToDraw ) {
		return;
	}

	UpdateUBOs();

	// Record Command Buffer
	if ( activeRenderList[ imageIndex ] != lastRenderList[ imageIndex ] )
		RecordCommandBuffer();

	VkSemaphore waitSemaphores[] = { vulkanSystem->imageAvailableSemaphores[ currentFrame ] };
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffers[ imageIndex ];

	VkSemaphore signalSemaphores[] = { vulkanSystem->renderFinishedSemaphores[ currentFrame ] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences( vulkanSystem->device, 1, &vulkanSystem->inFlightFences[ currentFrame ] );

	if ( vkQueueSubmit( vulkanSystem->graphicsQueue, 1, &submitInfo, vulkanSystem->inFlightFences[ currentFrame ] ) != VK_SUCCESS ) {
		Log::PrintlnWarn( "[Vulkan]Queue submit failed!" );
		return;
	}

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &vulkanSystem->swapChainKHR;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	if ( vkQueuePresentKHR( vulkanSystem->presentQueue, &presentInfo ) != VK_SUCCESS ) {
		Log::PrintlnWarn( "[Vulkan]Queue present failed!" );
		return;
	}

	lastRenderList[ imageIndex ] = activeRenderList[ imageIndex ];
	activeRenderList[ imageIndex ].clear();
		
	currentFrame = ( currentFrame + 1 ) % MAX_FRAMES_IN_FLIGHT;

	vkWaitForFences( vulkanSystem->device, 1, &vulkanSystem->inFlightFences[ currentFrame ], VK_TRUE, std::numeric_limits< uint64_t >::max() );

	if ( vkAcquireNextImageKHR( vulkanSystem->device, vulkanSystem->swapChainKHR, std::numeric_limits< uint64_t >::max(), vulkanSystem->imageAvailableSemaphores[ currentFrame ], VK_NULL_HANDLE, &imageIndex ) != VK_SUCCESS ) {
		Log::PrintlnWarn( "[Vulkan]Failed to acquire swap chain image" );
		imageAcquireNeeded = true;
	}
}

void RenderSystem::UpdateUBOs()
{
	for ( const auto &renderInfo : activeRenderList[ imageIndex ] )
	{
		auto mesh = renderInfo.mesh;
		auto material = Material::ToMaterial( mesh->GetMaterial() );
		auto shader = material->GetShader();
		MVP mvp = {
			renderInfo.modelMat,
			renderView.viewMatrix,
			renderView.projectionMatrix
		};

		shader->Update( imageIndex, mvp, mesh );
	}
}

void RenderSystem::RecordCommandBuffer()
{
	Log::PrintlnColor( fmt::color::blue, "Recording command buffer!" );
	auto &commandBuffer = commandBuffers[ imageIndex ];

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	beginInfo.pInheritanceInfo = nullptr; // Optional

	if ( vkBeginCommandBuffer( commandBuffer, &beginInfo ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create command buffers" );
	}

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = vulkanSystem->renderPass;
	renderPassInfo.framebuffer = vulkanSystem->swapChainFramebuffers[ imageIndex ];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = vulkanSystem->swapChainExtent;

	std::array< VkClearValue, 2 > clearValues;
	clearValues[ 0 ].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[ 1 ].depthStencil = { 1.0f, 0 };

	renderPassInfo.clearValueCount = static_cast< uint32_t >( clearValues.size() );
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass( commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
	for ( const auto &renderInfo : activeRenderList[ imageIndex ] )
	{
		const auto mesh = renderInfo.mesh;

		if ( !mesh )
			continue;

		const VkBuffer VertexBuffer = mesh->GetVertexBuffer();
		const VkBuffer IndexBuffer = mesh->GetIndexBuffer();

		if ( VertexBuffer != VK_NULL_HANDLE ) {
			auto material = Material::ToMaterial( mesh->GetMaterial() );
			auto shader = material->GetShader();

			vkCmdBindPipeline( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipeline() );
			vkCmdBindDescriptorSets( commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, shader->GetPipelineLayout(), 0, 1, &mesh->descriptorSets[ imageIndex ], 0, nullptr );

			const VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers( commandBuffer, 0, 1, &VertexBuffer, &offset );

			if ( IndexBuffer != VK_NULL_HANDLE ) {
				vkCmdBindIndexBuffer( commandBuffer, IndexBuffer, 0, VK_INDEX_TYPE_UINT32 );
				vkCmdDrawIndexed( commandBuffer, mesh->GetIndexCount(), 1, 0, 0, 0 );
			}
			else {
				vkCmdDraw( commandBuffer, mesh->GetVertexCount(), 1, 0, 0 );
			}
		}
	}

	vkCmdEndRenderPass( commandBuffer );

	if ( vkEndCommandBuffer( commandBuffer ) != VK_SUCCESS ) {
		engine->Error( "[Vulkan]Failed to create command buffers" );
	}
}

void RenderSystem::ClearRenderLists()
{
	for ( auto &renderList : activeRenderList )
		renderList.clear();
	for ( auto &renderList : lastRenderList )
		renderList.clear();
}