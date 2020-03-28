#include "wireshader.hpp"
#include "vertex.hpp"
#include "renderer.hpp"
#include "glm/gtx/transform.hpp"
#include "material.hpp"

void WireShader::InitMaterial( Material &material )
{
	
}

void WireShader::InitMesh( Mesh *mesh )
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

	mesh->ubos.resize( (size_t)Uniforms::Count );
	mesh->ubos[ (size_t)Uniforms::MVP ] = make_unique< UBO >( vulkanSystem, sizeof( MVP ) );

	auto ubo_mvp = mesh->ubos[ (size_t)Uniforms::MVP ].get();

	for ( uint32_t imageIndex = 0; imageIndex < vulkanSystem->numSwapChainImages; ++imageIndex )
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = ubo_mvp->uniformBuffer[ imageIndex ];
		bufferInfo.offset = 0;
		bufferInfo.range = ubo_mvp->BufferSize();

		std::array< VkWriteDescriptorSet, 1 > descriptorWrites = {};

		descriptorWrites[ 0 ].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[ 0 ].dstSet = mesh->descriptorSets[ imageIndex ];
		descriptorWrites[ 0 ].dstBinding = 0;
		descriptorWrites[ 0 ].dstArrayElement = 0;
		descriptorWrites[ 0 ].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[ 0 ].descriptorCount = 1;
		descriptorWrites[ 0 ].pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets( vulkanSystem->device, static_cast< uint32_t >( descriptorWrites.size() ), descriptorWrites.data(), 0, nullptr );
	}
}

void WireShader::Update( const uint32_t imageIndex, const MVP &mvp, Mesh *mesh )
{
	auto ubo_mvp = mesh->ubos[ (size_t)Uniforms::MVP ]->uniformBufferAllocation[ imageIndex ];

	if ( ubo_mvp != VK_NULL_HANDLE )
	{
		void *pData = nullptr;
		vulkanSystem->VmaMapMemory( ubo_mvp, &pData );
			std::memcpy( pData, &mvp, sizeof( mvp ) );
		vulkanSystem->VmaUnmapMemory( ubo_mvp );
	}
}

void WireShader::InitVertexInputBindingDescriptions()
{
	const VertexLayout vertexLayout = GetVertexLayout();
	vertexInputBindingDescriptions.resize( 1 );
	vertexInputBindingDescriptions[ 0 ] = vertexLayout.ToInputBindingDescription( 0 );
}

void WireShader::InitVertexInputAttributeDescriptions()
{
	const VertexLayout vertexLayout = GetVertexLayout();
	vertexInputAttributeDescriptions = vertexLayout.ToInputAttributeDescriptions( 0 );
}

void WireShader::CreateDescriptorSetLayout()
{
	std::vector< VkDescriptorSetLayoutBinding > bindings;

	VkDescriptorSetLayoutBinding uboLayoutBinding = {};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

	bindings.resize( 1 );
	bindings[ 0 ] = uboLayoutBinding;

	VkDescriptorSetLayoutCreateInfo layoutInfo = {};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast< uint32_t >( bindings.size() );
	layoutInfo.pBindings = bindings.data();

	vulkanSystem->CreateDescriptorSetLayout( &layoutInfo, nullptr, &descriptorSetLayout );
}

void WireShader::CreateGraphicsPipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	vulkanSystem->CreatePipelineLayout( &pipelineLayoutInfo, nullptr, &pipelineLayout );
}

void WireShader::CreateGraphicsPipeline()
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

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = static_cast< uint32_t >( vertexInputBindingDescriptions.size() );
	vertexInputInfo.pVertexBindingDescriptions = vertexInputBindingDescriptions.data();

	vertexInputInfo.vertexAttributeDescriptionCount = static_cast< uint32_t >( vertexInputAttributeDescriptions.size() );
	vertexInputInfo.pVertexAttributeDescriptions = vertexInputAttributeDescriptions.data();

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
	rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_FRONT_BIT; // TODO: Revisit this
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