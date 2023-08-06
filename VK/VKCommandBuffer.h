#include "VKInclude.h"

#include <queue>
#include <unordered_map>

MORISA_NAMESPACE_BEGIN

class VKRenderPass;
class VKBuffer;
class VKPipelineState;
class VKDescriptor;
class VKImage;
class VKUniform;

class VKCommandBuffer
{
public:
	VKCommandBuffer();
	~VKCommandBuffer();
	void Begin();
	void End();

	void Submit(
		VkFence submitFence = VK_NULL_HANDLE,
		VkSemaphore waitSemaphore = VK_NULL_HANDLE,
		VkSemaphore signalSemaphore = VK_NULL_HANDLE,
		VkPipelineStageFlags waitStage = 0);

	// Call each frame once
	void Recycle();

	void BeginRenderPass(
		VKRenderPass* renderPass, 
		const VkRect2D& renderArea);

	void EndRenderPass();

	void SetViewport(
		const VkViewport& viewport, 
		const VkRect2D& scissor);

	void BindPipeline(
		VkPipelineBindPoint bindPoint,
		VkPipeline pipeline);

	void BindVertexBuffer(VKBuffer* buffer);

	void BindIndexBuffer(VKBuffer* buffer, VkIndexType indexType);
	
	void BindDescriptorSets(VKPipelineState* state, VKUniform* uniform);

	void Draw(
		uint32_t vertexCount,
		uint32_t instanceCount,
		uint32_t firstVertex,
		uint32_t firstInstance);

	void DrawIndexed(
		uint32_t indexCount,
		uint32_t instanceCount,
		uint32_t firstIndex,
		uint32_t vertexOffset,
		uint32_t firstInstance);

	void CopyBuffer(VKBuffer* src, VKBuffer* dst);

	void CopyBufferToImage(VKBuffer* src, VKImage* dst);

	void BlitMipImage(VKImage* image, uint32_t curLevel);

	void TransitionLayout(VKImage* image, VkImageLayout oldLayout, VkImageLayout newLayout);

	void TransitionLayout(VKImage* image, VkImageLayout oldLayout, VkImageLayout newLayout, 
		uint32_t baseLevel, uint32_t levelCount);
private:									 
	VkCommandBuffer GetFreeCommandBuffer();	
private:
	VkQueue _submitQueue;
	VkCommandPool _commandPool;
	VkCommandBuffer _cmd;
	bool _cuurentInRecording;
	// TODO thread safety
	std::queue<VkCommandBuffer> _freePool;
	struct CommandBufferUsed
	{
		VkCommandBuffer commandBuffer;
		VkFence fence;
	};
	std::queue<CommandBufferUsed> _usedPool;
};



MORISA_NAMESPACE_END
