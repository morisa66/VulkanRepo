#include "VKPipelineState.h"
#include "VKContext.h"
#include "VKUniform.h"
#include "VKMesh.h"
#include "VKDescriptor.h"
#include <array>

MORISA_NAMESPACE_BEGIN

VKPipelineState::VKPipelineState():
	_layout(VK_NULL_HANDLE)
{
	ConfigureBaseState();
}

VKPipelineState::~VKPipelineState()
{
	VK_VALID_DESTROY(_layout, vkDestroyPipelineLayout, Device(), _layout, nullptr);
}

void VKPipelineState::UpdateViewport(const VkExtent2D& windowSize)
{
	_viewport.width = (float)windowSize.width;
	_viewport.height = (float)windowSize.height;
	_scissor.extent.width = windowSize.width;
	_scissor.extent.height = windowSize.height;
}

void VKPipelineState::AddAttachmentState(bool useAlphaBlend)
{
	VkPipelineColorBlendAttachmentState attachmentState{};
	attachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	if (useAlphaBlend)
	{
		//outColor= currentAlpha * currentColor + (1 - currentAlpha) * backgroundColor;
		//outAlpha = currentAlpha;
		attachmentState.blendEnable = VK_TRUE;
		attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	else
	{
		attachmentState.blendEnable = VK_FALSE;
		attachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		attachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		attachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		attachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		attachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		attachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
	}
	_attachmentStates.emplace_back(attachmentState);
}

void VKPipelineState::ConfigureVertexInput(VKMesh* mesh)
{
	VK_STRUCT_INIT(_vertexInputState, VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO);
	_vertexInputState.vertexBindingDescriptionCount = mesh->GetBindingDescriptions().size();
	_vertexInputState.pVertexBindingDescriptions = mesh->GetBindingDescriptions().data();
	_vertexInputState.vertexAttributeDescriptionCount = mesh->GetAttributeDescriptions().size();
	_vertexInputState.pVertexAttributeDescriptions = mesh->GetAttributeDescriptions().data();
}

void VKPipelineState::ConfigureLayout(VKUniform* localUniform)
{
	std::array<VkDescriptorSetLayout, 2> layouts
	{
		// Global Uniform is always set
		Context()->GlobalUniform()->DescriptorSet()->Layout(),
		localUniform->DescriptorSet()->Layout(),
	};

	VK_STRUCT_CREATE(VkPipelineLayoutCreateInfo, pipelineLayoutInfo, VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
	pipelineLayoutInfo.setLayoutCount = layouts.size();
	pipelineLayoutInfo.pSetLayouts = layouts.data();
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VK_RESULT_CHECK(vkCreatePipelineLayout, Device(), &pipelineLayoutInfo, nullptr, &_layout);
}

void VKPipelineState::ConfigureSamples(VkSampleCountFlagBits samples)
{
	_multisampleState.rasterizationSamples = samples;
}

void VKPipelineState::ConfigureBaseState()
{
	VK_STRUCT_INIT(_inputAssemblyState, VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO);
	_inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	_inputAssemblyState.primitiveRestartEnable = VK_FALSE;

	VK_STRUCT_INIT(_tessellationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
	_tessellationState.patchControlPoints = 0;

	VK_STRUCT_INIT(_viewportState, VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO);
	memset(&_viewport, 0, sizeof(VkViewport));
	_viewport.maxDepth = 1.0f;
	memset(&_scissor, 0, sizeof(VkRect2D));
	UpdateViewport(Context()->WindowSize());
	_viewportState.viewportCount = 1;
	_viewportState.pViewports = &_viewport;
	_viewportState.scissorCount = 1;
	_viewportState.pScissors = &_scissor;

	VK_STRUCT_INIT(_rasterizationState, VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO);
	_rasterizationState.depthClampEnable = VK_FALSE;
	_rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	_rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	_rasterizationState.cullMode = VK_CULL_MODE_BACK_BIT;
	_rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	_rasterizationState.depthBiasEnable = VK_FALSE;
	_rasterizationState.depthBiasConstantFactor = 0.0f;
	_rasterizationState.depthBiasClamp = 0.0f;
	_rasterizationState.depthBiasSlopeFactor = 0.0f;
	_rasterizationState.lineWidth = 1.0f;

	VK_STRUCT_INIT(_multisampleState, VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO);
	_multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	_multisampleState.sampleShadingEnable = VK_FALSE;
	_multisampleState.minSampleShading = 1.0f;
	_multisampleState.pSampleMask = nullptr;
	_multisampleState.alphaToCoverageEnable = VK_FALSE;
	_multisampleState.alphaToOneEnable = VK_FALSE;

	VK_STRUCT_INIT(_depthStencilState, VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO);
	_depthStencilState.depthTestEnable = VK_TRUE;
	_depthStencilState.depthWriteEnable = VK_TRUE;
	_depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	_depthStencilState.depthBoundsTestEnable = VK_FALSE;
	_depthStencilState.stencilTestEnable = VK_FALSE;
	_depthStencilState.front = {};
	_depthStencilState.back = {};
	_depthStencilState.minDepthBounds = 0.0f;
	_depthStencilState.maxDepthBounds = 1.0f;

	VK_STRUCT_INIT(_colorBlendState, VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO);
	AddAttachmentState(false);
	_colorBlendState.logicOpEnable = VK_FALSE;
	_colorBlendState.logicOp = VK_LOGIC_OP_COPY;
	_colorBlendState.attachmentCount = _attachmentStates.size();
	_colorBlendState.pAttachments = _attachmentStates.data();
	_colorBlendState.blendConstants[0] = 0.0f;
	_colorBlendState.blendConstants[1] = 0.0f;
	_colorBlendState.blendConstants[2] = 0.0f;
	_colorBlendState.blendConstants[3] = 0.0f;

	VK_STRUCT_INIT(_dynamicState, VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO);
	_dynamicState.dynamicStateCount = _dynamic.size();
	_dynamicState.pDynamicStates = _dynamic.data();
}


MORISA_NAMESPACE_END