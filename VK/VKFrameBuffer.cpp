#include "VKFrameBuffer.h"
#include "VKContext.h"
#include "VKRenderPass.h"
#include "VKImage.h"

#include <algorithm>

MORISA_NAMESPACE_BEGIN


VKFramebuffer::VKFramebuffer(VKRenderPass* renderPass):
	_currentIndex(0)
{
	const VkExtent2D& size = renderPass->Info().scope.extent;
	const MVector<Attachment>& attachments = renderPass->Info().attachments;
	const uint32_t attachmentSize = attachments.size();

	MVector<Attachment>::const_iterator swapChainPos = std::find_if(attachments.cbegin(), attachments.cend(),
		[](const Attachment& attachment) 
		{
			return attachment.renderImage == VKImage::SwapChainImage();
		}) ;

	MVector<VkImageView> attachmentViews(attachmentSize);
	for (uint32_t i = 0; i < attachmentSize; ++i)
	{
		attachmentViews[i] = attachments[i].View();
	}

	VK_STRUCT_CREATE(VkFramebufferCreateInfo, createInfo, VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO);
	createInfo.renderPass = renderPass->RenderPass();
	createInfo.width = size.width;
	createInfo.height = size.height;
	createInfo.layers = 1;
	createInfo.attachmentCount = attachmentViews.size();
	createInfo.pAttachments = attachmentViews.data();
	if (swapChainPos == attachments.cend())
	{
		_framebuffers.resize(1, VK_NULL_HANDLE);
		VK_RESULT_CHECK(vkCreateFramebuffer, Device(), &createInfo, nullptr, &_framebuffers[0]);
	}
	else
	{
		const uint32_t swapChainIndex = std::distance(attachments.cbegin(), swapChainPos);
		const VKSwapChainImageInfo& info = Context()->GetSwapChainImageInfo();
		_framebuffers.resize(info.views.size(), VK_NULL_HANDLE);
		for (int i = 0; i < _framebuffers.size(); ++i)
		{
			attachmentViews[swapChainIndex] = info.views[i];
			VK_RESULT_CHECK(vkCreateFramebuffer, Device(), &createInfo, nullptr, &_framebuffers[i]);
		}
	}
}

VKFramebuffer::~VKFramebuffer()
{
	for (VkFramebuffer framebuffer : _framebuffers)
	{
		VK_VALID_DESTROY(framebuffer, vkDestroyFramebuffer, Device(), framebuffer, nullptr);
	}
}

MORISA_NAMESPACE_END


