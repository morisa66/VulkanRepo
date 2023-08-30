#include "MRenderPass.h"
#include "VK/VKRenderPass.h"
#include "VK/VKContext.h"
#include "VK/VKCommandBuffer.h"
#include "VK/VKPipelineState.h"
#include "VK/VKPipeline.h"
#include "Runtime/Graphics/Material.h"
#include "VK/VKMesh.h"
#include "VK/VKDescriptor.h"
#include "VK/VKUniform.h"

MORISA_NAMESPACE_BEGIN

MRenderPass::MRenderPass()
{
}

MRenderPass::~MRenderPass()
{
	for (MRenderNode* node : _renderNodes)
	{
		MORISA_DELETE(node);
	}

	MORISA_DELETE(_nativePass);
}

void MRenderPass::Execute()
{
	// TODO
	// Sort to render nearby first
	
	// Make sure commandBuffer is recording
	VKCommandBuffer* commandBuffer = Context()->CommandBuffer();
	
	commandBuffer->BeginRenderPass(_nativePass);

	for (MRenderNode* node : _renderNodes)
	{
		RenderNode(node);
	}

	commandBuffer->EndRenderPass();
}

void MRenderPass::ResizeRenderTarget(const VkExtent2D& extent)
{
	_extent = extent;
	_nativePass->Info().scope.extent = extent;
}

void MRenderPass::RenderNode(MRenderNode* node)
{
	VKContext* context = Context();
	VKCommandBuffer* commandBuffer = context->CommandBuffer();
	VKDescriptorManager* descriptorManager = context->DescriptorManager();

	VKPipelineKey pipelineKey;
	pipelineKey.program = node->material->Shader()->Program();
	pipelineKey.state = node->state;
	pipelineKey.pass = _nativePass;
	VkPipeline pipeline = context->PipelineCache()->GetCachePipeline(pipelineKey);

	// TODO if Pipeline not changed, don't bind it twice
	commandBuffer->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

	for (int i = 0; i < node->mesh->GetMeshCount(); ++i)
	{
		const VKMeshData& meshData = node->mesh->GetMeshData(i);
		if (node->info.needNodeImage && meshData.imageIndices.size() > 0)
		{
			commandBuffer->BindDescriptorSet(node->state, node->uniform->DescriptorSet(meshData.imageIndices[0])->Set());
		}
		else
		{
			commandBuffer->BindDescriptorSet(node->state, node->uniform->DescriptorSet()->Set());
		}
		commandBuffer->BindVertexBuffer(meshData.vertexBuffer);
		if (meshData.drawType == kMDrawTypeDraw)
		{
			commandBuffer->Draw(meshData.vertexCount, 1, 0, 0);
		}
		else
		{
			commandBuffer->BindIndexBuffer(meshData.indexBuffer, VK_INDEX_TYPE_UINT32);
			commandBuffer->DrawIndexed(meshData.indexCount, 1, 0, 0, 0);
		}
	}
}

MRenderNode* MRenderPass::GenerateNode()
{
	MRenderNode* node = MORISA_NEW(MRenderNode);
	_renderNodes.emplace_back(node);
	return node;
}

MORISA_NAMESPACE_END

