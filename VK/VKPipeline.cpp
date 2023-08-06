#include "VKPipeline.h"
#include "VKContext.h"
#include "VKPipelineState.h"
#include "VKRenderPass.h"
#include "VKGpuProgram.h"


#include <algorithm>

MORISA_NAMESPACE_BEGIN

VKPipelineCache::VKPipelineCache()
{
}

VKPipelineCache::~VKPipelineCache()
{
	for (VKPipelineNode node = _pipelineCache.begin(); node != _pipelineCache.end(); node++)
	{
		VK_VALID_DESTROY(node->second, vkDestroyPipeline, Device(), node->second, nullptr);
	}
	_pipelineCache.clear();
}

VkPipeline VKPipelineCache::GetCachePipeline(const VKPipelineKey& key)
{
	VKPipelineNode node = _pipelineCache.find(key);
	if (node != _pipelineCache.cend())
	{
		return node->second;
	}
	VK_STRUCT_CREATE(VkGraphicsPipelineCreateInfo, createInfo, VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO);
	createInfo.stageCount = VKShaderTypeCount;
	createInfo.pStages = key.program->GetStageInfo();
	createInfo.pVertexInputState = &key.state->_vertexInputState;
	createInfo.pInputAssemblyState = &key.state->_inputAssemblyState;
	createInfo.pTessellationState = &key.state->_tessellationState;
	createInfo.pViewportState = &key.state->_viewportState;
	createInfo.pRasterizationState = &key.state->_rasterizationState;
	createInfo.pMultisampleState = &key.state->_multisampleState;
	createInfo.pDepthStencilState = &key.state->_depthStencilState;
	createInfo.pColorBlendState = &key.state->_colorBlendState;
	createInfo.pDynamicState = &key.state->_dynamicState;
	createInfo.layout = key.state->_layout;
	createInfo.renderPass = key.pass->RenderPass();
	// TODO Multi subpass
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = UINT32_MAX;

	VkPipeline pipeline = VK_NULL_HANDLE;

	VK_RESULT_CHECK(vkCreateGraphicsPipelines, Device(), nullptr, 1, &createInfo, nullptr, &pipeline);

	// TODO Native Pipeline Cache	
	_pipelineCache.emplace(key , pipeline);

	return pipeline;
}

MORISA_NAMESPACE_END