#include "shader_staticmesh.hpp"
#include "vertex.hpp"
#include "renderer.hpp"
#include "glm/gtx/transform.hpp"
#include "texturesystem.hpp"
#include "material.hpp"

VkDescriptorPool Shader_StaticMesh::CreateDescriptorPool() const
{
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

	std::array< VkDescriptorPoolSize, 3 > poolSizes;
	poolSizes[ 0 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[ 0 ].descriptorCount = vulkanSystem->numSwapChainImages;
	poolSizes[ 1 ].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[ 1 ].descriptorCount = vulkanSystem->numSwapChainImages;
	poolSizes[ 2 ].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[ 2 ].descriptorCount = vulkanSystem->numSwapChainImages;

	VkDescriptorPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast< uint32_t >( poolSizes.size() );
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = vulkanSystem->numSwapChainImages;

	vulkanSystem->CreateDescriptorPool( &poolInfo, nullptr, &descriptorPool );

	return descriptorPool;
}

void Shader_StaticMesh::InitMaterial( Material &material )
{
}

void Shader_StaticMesh::InitMesh( Mesh *mesh )
{
	mesh->descriptorPool = CreateDescriptorPool();
	const std::vector< VkDescriptorSetLayout > layouts( vulkanSystem->numSwapChainImages, GetDescriptorSetLayout() );

	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = mesh->descriptorPool;
	allocInfo.descriptorSetCount = vulkanSystem->numSwapChainImages;
	allocInfo.pSetLayouts = layouts.data();

	mesh->descriptorSets.resize( vulkanSystem->numSwapChainImages );
	vulkanSystem->AllocateDescriptorSets( &allocInfo, mesh->descriptorSets.data() );

	auto material = mesh->GetMaterial();

	mesh->ubos.resize( (size_t)Uniforms::Count );
	mesh->ubos[ (size_t)Uniforms::MVP ] = make_unique< UBO >( vulkanSystem, sizeof( glm::mat4 ) );
	mesh->ubos[ (size_t)Uniforms::LightState ] = make_unique< UBO >( vulkanSystem, sizeof( glm::vec4 ) );

	auto ubo_mvp = mesh->ubos[ (size_t)Uniforms::MVP ].get();
	auto ubo_lightState = mesh->ubos[ (size_t)Uniforms::LightState ].get();
	auto diffuse = material->GetTexture( "diffuse" );

	if ( !diffuse ) {
		Log::Println( "Failed to load \"diffuse\" texture for shader {}", GetShaderName() );
	}

	for ( uint32_t imageIndex = 0; imageIndex < vulkanSystem->numSwapChainImages; ++imageIndex )
	{
		VkDescriptorBufferInfo mvpBufferInfo = {};
		mvpBufferInfo.buffer = ubo_mvp->uniformBuffer[ imageIndex ];
		mvpBufferInfo.offset = 0;
		mvpBufferInfo.range = ubo_mvp->BufferSize();

		VkDescriptorBufferInfo lightStateBufferInfo = {};
		lightStateBufferInfo.buffer = ubo_lightState->uniformBuffer[ imageIndex ];
		lightStateBufferInfo.offset = 0;
		lightStateBufferInfo.range = ubo_lightState->BufferSize();

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		if ( diffuse ) {
			imageInfo.imageView = diffuse->GetImageView();
			imageInfo.sampler = diffuse->GetSampler();
		}

		std::array< VkWriteDescriptorSet, 3 > descriptorWrites = {};

		descriptorWrites[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[ 0 ].dstSet = mesh->descriptorSets[ imageIndex ];
		descriptorWrites[ 0 ].dstBinding = 0;
		descriptorWrites[ 0 ].dstArrayElement = 0;
		descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[ 0 ].descriptorCount = 1;
		descriptorWrites[ 0 ].pBufferInfo = &mvpBufferInfo;

		descriptorWrites[ 1 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[ 1 ].dstSet = mesh->descriptorSets[ imageIndex ];
		descriptorWrites[ 1 ].dstBinding = 1;
		descriptorWrites[ 1 ].dstArrayElement = 0;
		descriptorWrites[ 1 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[ 1 ].descriptorCount = 1;
		descriptorWrites[ 1 ].pBufferInfo = &lightStateBufferInfo;

		descriptorWrites[ 2 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[ 2 ].dstSet = mesh->descriptorSets[ imageIndex ];
		descriptorWrites[ 2 ].dstBinding = 2;
		descriptorWrites[ 2 ].dstArrayElement = 0;
		descriptorWrites[ 2 ].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[ 2 ].descriptorCount = 1;
		descriptorWrites[ 2 ].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets( vulkanSystem->device, static_cast< uint32_t >( descriptorWrites.size() ), descriptorWrites.data(), 0, nullptr );
	}
}

void Shader_StaticMesh::Update( const uint32_t imageIndex, const MVP &mvp, Mesh *mesh )
{
	auto ubo_mvp = mesh->ubos[ (size_t)Uniforms::MVP ]->uniformBufferAllocation[ imageIndex ];
	auto ubo_lightState = mesh->ubos[ (size_t)Uniforms::LightState ]->uniformBufferAllocation[ imageIndex ];

	const glm::mat4 modelToClip = mvp.proj * mvp.view * mvp.model;

	if ( ubo_mvp != VK_NULL_HANDLE )
	{
		void *pData = nullptr;
		vulkanSystem->VmaMapMemory( ubo_mvp, &pData );
			std::memcpy( pData, &modelToClip, sizeof( modelToClip ) );
		vulkanSystem->VmaUnmapMemory( ubo_mvp );
	}

	const glm::vec4 ambientLight = { 1.0f, 1.0f, 1.0f, 1.0f };

	if ( ubo_lightState != VK_NULL_HANDLE )
	{
		void *pData = nullptr;
		vulkanSystem->VmaMapMemory( ubo_lightState, &pData );
			std::memcpy( pData, &ambientLight, sizeof( ambientLight ) );
		vulkanSystem->VmaUnmapMemory( ubo_lightState );
	}
}

void Shader_StaticMesh::CreateDescriptorSetLayout()
{
	std::vector< VkDescriptorSetLayoutBinding > bindings;

	VkDescriptorSetLayoutBinding mvpLayoutBinding = {};
	mvpLayoutBinding.binding = 0;
	mvpLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	mvpLayoutBinding.descriptorCount = 1;
	mvpLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	mvpLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding lightStateLayoutBinding = {};
	lightStateLayoutBinding.binding = 1;
	lightStateLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	lightStateLayoutBinding.descriptorCount = 1;
	lightStateLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	lightStateLayoutBinding.pImmutableSamplers = nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding = {};
	samplerLayoutBinding.binding = 2;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers = nullptr;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	bindings.resize( 3 );
	bindings[ 0 ] = mvpLayoutBinding;
	bindings[ 1 ] = lightStateLayoutBinding;
	bindings[ 2 ] = samplerLayoutBinding;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size() );
	layoutInfo.pBindings = bindings.data();

	vulkanSystem->CreateDescriptorSetLayout( &layoutInfo, nullptr, &descriptorSetLayout );
}

void Shader_StaticMesh::CreateGraphicsPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	vulkanSystem->CreatePipelineLayout( &pipelineLayoutInfo, nullptr, &pipelineLayout );
}

void Shader_StaticMesh::CreateGraphicsPipeline()
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = shaderModules[ static_cast< size_t >( ShaderType::Vertex ) ];
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = shaderModules[ static_cast< size_t >( ShaderType::Fragment ) ];
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	const VertexLayout vertexLayout = GetVertexLayout();
	VkVertexInputBindingDescription bindingDescription = vertexLayout.ToInputBindingDescription( 0 );
	std::vector< VkVertexInputAttributeDescription > attribDescriptions = vertexLayout.ToInputAttributeDescriptions( 0 );

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;

	vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( attribDescriptions.size() );
	vertexInputInfo.pVertexAttributeDescriptions = attribDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast< float >( vulkanSystem->swapChainExtent.width );
	viewport.height = static_cast< float >( vulkanSystem->swapChainExtent.height );
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = vulkanSystem->swapChainExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	//rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; // TODO: Revisit this
	//rasterizer.cullMode = VK_CULL_MODE_NONE;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE; // TODO: Revisit this
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	depthStencilState = {};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.minDepthBounds = 0.0f; // Optional
	depthStencilState.maxDepthBounds = 1.0f; // Optional
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {}; // Optional
	depthStencilState.back = {}; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

	// Alpha Blending
	colorBlendAttachment.blendEnable = VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[ 0 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 1 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 2 ] = 0.0f; // Optional
	colorBlending.blendConstants[ 3 ] = 0.0f; // Optional

	VkGraphicsPipelineCreateInfo pipelineInfo = {};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = static_cast< uint32_t >( ShaderType::Max );
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = vulkanSystem->renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	vulkanSystem->CreateGraphicsPipelines( VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline );
}