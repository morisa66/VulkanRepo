#include "VKCommandBuffer.h"
#include "VKContext.h"
#include "VKRenderPass.h"
#include "VKFrameBuffer.h"
#include "VKBuffer.h"
#include "VKDescriptor.h"
#include "VKPipelineState.h"
#include "VKImage.h"
#include "VKUniform.h"
#include "Utils/MathUtils.h"


#include <algorithm>
#include <array>

MORISA_NAMESPACE_BEGIN

VKCommandBuffer::VKCommandBuffer():
	_commandPool(VK_NULL_HANDLE)
	, _submitQueue(Context()->GraphicsQueue())
	, _cmd(VK_NULL_HANDLE)
	, _cuurentInRecording(false)
{
	VK_STRUCT_CREATE(VkCommandPoolCreateInfo, createInfo, VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
	// reset
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = Context()->GraphicsQueueIndex();

	VK_RESULT_CHECK(vkCreateCommandPool, Device(), &createInfo, nullptr, &_commandPool);
}

VKCommandBuffer::~VKCommandBuffer()
{
	VK_VALID_DESTROY(_commandPool, vkDestroyCommandPool, Device(), _commandPool, nullptr);
}


void VKCommandBuffer::Begin()
{
	if (_cuurentInRecording)
	{
		MORISA_LOG("Try to begin the commandbuffer twice\n");
		return;
	}
	VkCommandBufferBeginInfo commandBufferBeginInfo{};
	VK_STRUCT_CREATE(VkCommandBufferBeginInfo, beginInfo, VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO);
	beginInfo.pInheritanceInfo = nullptr;
	_cmd = GetFreeCommandBuffer();
	VK_RESULT_CHECK(vkBeginCommandBuffer, _cmd, &beginInfo);
	_cuurentInRecording = true;
}

void VKCommandBuffer::End()
{
	if (_cuurentInRecording)
	{
		VK_RESULT_CHECK(vkEndCommandBuffer, _cmd);
		_cuurentInRecording = false;
	}
	else
	{
		MORISA_LOG("Try to end the commandbuffer which is not begin\n");
	}
}

void VKCommandBuffer::Submit(
	VkFence submitFence,
	VkSemaphore waitSemaphore, 
	VkSemaphore signalSemaphore, 
	VkPipelineStageFlags waitStage)
{
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.pNext = nullptr;
	submitInfo.waitSemaphoreCount = waitSemaphore == VK_NULL_HANDLE ? 0 : 1;
	submitInfo.pWaitSemaphores = &waitSemaphore;
	submitInfo.pWaitDstStageMask = &waitStage;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_cmd;
	submitInfo.signalSemaphoreCount = signalSemaphore == VK_NULL_HANDLE  ? 0 : 1;
	submitInfo.pSignalSemaphores = &signalSemaphore;

	VK_RESULT_CHECK(vkQueueSubmit, _submitQueue, 1, &submitInfo, submitFence);

	if (submitFence != VK_NULL_HANDLE)
	{
		_usedPool.push(CommandBufferUsed{ _cmd, submitFence });
	}
}

void VKCommandBuffer::Recycle()
{
	while (!_usedPool.empty()
		&& vkGetFenceStatus(Device(), _usedPool.front().fence) == VK_SUCCESS)
	{
		VkCommandBuffer cmd = _usedPool.front().commandBuffer;
		//vkResetCommandBuffer(commandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkResetCommandBuffer(_cmd, 0);
		_usedPool.pop();
		_freePool.push(_cmd);
	}
}

void VKCommandBuffer::BeginRenderPass(VKRenderPass* renderPass)
{
	const VkRect2D& renderArea = renderPass->Info().scope;
	BeginRenderPass(renderPass, renderArea);
	VkViewport viewport{ 0.0f, 0.0f, (float)renderArea.extent.width, (float)renderArea.extent.height, 0.0f, 1.0f };
	SetViewport(viewport, renderArea);
}

void VKCommandBuffer::BeginRenderPass(
	VKRenderPass* renderPass, 
	const VkRect2D& renderArea)
{
	VkRenderPassBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	beginInfo.pNext = nullptr;
	beginInfo.renderPass = renderPass->RenderPass();
	beginInfo.framebuffer = renderPass->FrameBuffer()->GetCurrentBuffer();
	beginInfo.renderArea = renderArea;
	beginInfo.clearValueCount = renderPass->Info().clearValues.size();
	beginInfo.pClearValues = renderPass->Info().clearValues.data();
	vkCmdBeginRenderPass(_cmd, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VKCommandBuffer::EndRenderPass()
{
	vkCmdEndRenderPass(_cmd);
}

void VKCommandBuffer::SetViewport(const VkViewport& viewport, const VkRect2D& scissor)
{
	vkCmdSetViewport(_cmd, 0, 1, &viewport);
	vkCmdSetScissor(_cmd, 0, 1, &scissor);
}

void VKCommandBuffer::BindPipeline(VkPipelineBindPoint bindPoint, VkPipeline pipeline)
{
	vkCmdBindPipeline(_cmd, bindPoint, pipeline);
}

void VKCommandBuffer::BindVertexBuffer(VKBuffer* buffer)
{
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(_cmd, 0, 1, buffer->AccessPtr(), &offset);
}

void VKCommandBuffer::BindIndexBuffer(VKBuffer* buffer, VkIndexType indexType)
{
	vkCmdBindIndexBuffer(_cmd, buffer->Access(), 0, indexType);
}

void VKCommandBuffer::BindDescriptorSet(VKPipelineState* state, VkDescriptorSet set)
{
	static std::array<VkDescriptorSet, 2> sets;
	sets[0] = Context()->GlobalUniform()->DescriptorSet()->Set();
	sets[1] = set;
	vkCmdBindDescriptorSets(_cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		state->Layout(), 0,
		sets.size(), sets.data(),
		0, nullptr);
}

void VKCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance)
{
	vkCmdDraw(_cmd, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VKCommandBuffer::DrawIndexed(
	uint32_t indexCount,
	uint32_t instanceCount,
	uint32_t firstIndex,
	uint32_t vertexOffset,
	uint32_t firstInstance)
{
	vkCmdDrawIndexed(_cmd, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VKCommandBuffer::CopyBuffer(VKBuffer* src, VKBuffer* dst)
{
	VkBufferCopy bufferCopy{};
	bufferCopy.srcOffset = 0; 
	bufferCopy.dstOffset = 0; 
	bufferCopy.size = std::min(src->Size(), dst->Size());
	vkCmdCopyBuffer(_cmd, src->Access(), dst->Access(), 1, & bufferCopy);
}

void VKCommandBuffer::CopyBufferToImage(VKBuffer* src, VKImage* dst)
{
	VkBufferImageCopy bufferImageCopy{};
	bufferImageCopy.bufferOffset = 0;
	bufferImageCopy.bufferRowLength = 0;
	bufferImageCopy.bufferImageHeight = 0;
	bufferImageCopy.imageSubresource = VkImageSubresourceLayers{ VK_IMAGE_ASPECT_COLOR_BIT , 0, 0, 1 };
	bufferImageCopy.imageOffset = VkOffset3D{ 0, 0, 0 };
	bufferImageCopy.imageExtent = VkExtent3D{ dst->Info().width, dst->Info().height, 1};

	vkCmdCopyBufferToImage(_cmd, src->Access(), dst->Access(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
}

void VKCommandBuffer::BlitMipImage(VKImage* image, uint32_t curLevel)
{
	VkImageBlit imageBlit{};
	uint32_t mipLevel = curLevel + 1;
	int srcWidth = Max(1, ((int)image->Info().width) >> curLevel);
	int srcHeight = Max(1, ((int)image->Info().height) >> curLevel);
	int dstWidth = Max(1, srcWidth >> 1);
	int dstHeight = Max(1, srcHeight >> 1);
	imageBlit.srcOffsets[0] = { 0, 0, 0 };
	imageBlit.srcOffsets[1] = { srcWidth, srcHeight, 1 };
	imageBlit.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, curLevel, 0, 1 };
	imageBlit.dstOffsets[0] = { 0, 0, 0 };
	imageBlit.dstOffsets[1] = { dstWidth, dstHeight, 1 };
	imageBlit.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, mipLevel, 0, 1 };

	vkCmdBlitImage(_cmd,
		image->Access(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		image->Access(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1, &imageBlit, VK_FILTER_LINEAR);
}

void VKCommandBuffer::TransitionLayout(VKImage* image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	TransitionLayout(image, oldLayout, newLayout, 0, image->Info().mipLevels);
}

void VKCommandBuffer::TransitionLayout(VKImage* image, VkImageLayout oldLayout, VkImageLayout newLayout, 
	uint32_t baseLevel, uint32_t levelCount)
{
	VkAccessFlags srcAccessMask = 0;
	VkAccessFlags dstAccessMask = 0;
	VkPipelineStageFlags srcStageMask = 0;
	VkPipelineStageFlags dstStageMask = 0;

	switch (oldLayout)
	{
		case VK_IMAGE_LAYOUT_UNDEFINED:
		{
			srcAccessMask = 0;
			srcStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
			srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		{
			srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			srcStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		default:
			break;
	}

	switch (newLayout)
	{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
		{
			dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
		{
			dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
		{
			dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			break;
		}
		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
		{
			dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT 
				| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		}
		default:
			break;
	}

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.pNext = nullptr;
	barrier.srcAccessMask = srcAccessMask;
	barrier.dstAccessMask = dstAccessMask;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	// Multi queue needed
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->Access();
	barrier.subresourceRange = VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT , baseLevel, levelCount, 0, 1};

	vkCmdPipelineBarrier(_cmd, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VKCommandBuffer::SetDepthBias(float constant, float clamp, float slope)
{
	vkCmdSetDepthBias(_cmd, constant, clamp, slope);
}

VkCommandBuffer VKCommandBuffer::GetFreeCommandBuffer()
{
	VkCommandBuffer cmd = VK_NULL_HANDLE;
	if (!_freePool.empty())
	{
		cmd = _freePool.front();
		_freePool.pop();
	}
	else
	{
		VkCommandBufferAllocateInfo allocateInfo{};
		allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocateInfo.pNext = nullptr;
		allocateInfo.commandPool = _commandPool;
		allocateInfo.commandBufferCount = 1;
		allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VK_RESULT_CHECK(vkAllocateCommandBuffers, Device(), &allocateInfo, &cmd);
	}
	return cmd;
}

MORISA_NAMESPACE_END