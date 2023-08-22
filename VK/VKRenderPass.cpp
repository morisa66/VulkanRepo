#include "VKRenderPass.h"
#include "VKContext.h"
#include "VKImage.h"
#include "VKFrameBuffer.h"
#include "Utils/MathUtils.h"

MORISA_NAMESPACE_BEGIN

VkFormat Attachment::Format() const
{
	return renderImage != VKImage::SwapChainImage() ? renderImage->Info().format : Context()->GetSwapChainImageInfo().format;
}

VkSampleCountFlagBits Attachment::Samples() const
{
	return  renderImage != VKImage::SwapChainImage() ? renderImage->Info().samples : VK_SAMPLE_COUNT_1_BIT;
}

VkImageView Attachment::View() const
{
	return renderImage != VKImage::SwapChainImage() ? renderImage->AccessView() : VK_NULL_HANDLE;
}

VkImageAspectFlags Attachment::Aspect() const
{
	return renderImage != VKImage::SwapChainImage() ? renderImage->Info().aspect : VK_IMAGE_ASPECT_COLOR_BIT;
}

VKRenderPass::VKRenderPass():
	_renderPass(VK_NULL_HANDLE)
	, _frameBuffer(nullptr)
{
}

VKRenderPass::~VKRenderPass()
{
	MORISA_DELETE(_frameBuffer);
	VK_VALID_DESTROY(_renderPass, vkDestroyRenderPass, Device(), _renderPass, nullptr);
}

void VKRenderPass::Create()
{
	FillAttachments();
	CreateRenderPass();

	_frameBuffer = MORISA_NEW(VKFramebuffer, this);
}

void VKRenderPass::FlushRenderImages(std::initializer_list<VKImage*> images)
{
	uint32_t i = 0;
	for (std::initializer_list<VKImage*>::iterator it = images.begin();
		it != images.end() && i < _info.attachments.size(); ++it, ++i)
	{
		_info.attachments[i].renderImage = const_cast<VKImage*>(*it);
	}
	MORISA_DELETE(_frameBuffer);
	_frameBuffer = MORISA_NEW(VKFramebuffer, this);
}

void VKRenderPass::FillAttachments()
{
	const uint32_t size = _info.attachments.size();
	_descriptions.resize(size);
	for (uint32_t i = 0; i < size; ++i)
	{
		const Attachment& attachment = _info.attachments[i];
		VkAttachmentDescription2& description = _descriptions[i];
		VK_STRUCT_INIT(description, VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2);
		description.format = attachment.Format();
		description.samples = attachment.Samples();
		description.loadOp = attachment.loadOp;
		description.storeOp = attachment.storeOp;
		description.stencilLoadOp = attachment.stencilloadOp;
		description.stencilStoreOp = attachment.stencilStoreOp;
		description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		description.finalLayout = attachment.finalLayout;
	}

	for (uint32_t index : _info.subpasses[0].colorIndices)
	{
		_colorReferences.emplace_back(VkAttachmentReference2
			{ 
			VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			nullptr,
			index, 
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			_info.attachments[index].Aspect()
			});
	}

	VkSubpassDescription2 subpass{};
	VK_STRUCT_INIT(subpass, VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2);
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = _colorReferences.size();
	subpass.pColorAttachments = _colorReferences.data();
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = nullptr;
	if (_info.subpasses[0].depthIndex != Subpass::INVALID_INDEX)
	{
		_depthReference = VkAttachmentReference2
		{ 
			VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			nullptr,
			_info.subpasses[0].depthIndex,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			_info.attachments[_info.subpasses[0].depthIndex].Aspect()
		};
		subpass.pDepthStencilAttachment = &_depthReference;
	}
	else
	{
		subpass.pDepthStencilAttachment = nullptr;
	}

	if (_info.subpasses[0].resolveIndex != Subpass::INVALID_INDEX)
	{
		_resolveReference = VkAttachmentReference2
		{
			VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			nullptr,
			_info.subpasses[0].resolveIndex,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			_info.attachments[_info.subpasses[0].resolveIndex].Aspect()
		};
		subpass.pResolveAttachments = &_resolveReference;
	}
	else
	{
		subpass.pResolveAttachments = nullptr;
	}

	if (_info.subpasses[0].depthResolveIndex != Subpass::INVALID_INDEX)
	{
		_depthStencilResolveReference = VkAttachmentReference2
		{
			VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			nullptr,
			_info.subpasses[0].depthResolveIndex,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
			_info.attachments[_info.subpasses[0].depthResolveIndex].Aspect()
		};
		_depthStencilResolve.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_DEPTH_STENCIL_RESOLVE;
		_depthStencilResolve.pNext = nullptr;
		_depthStencilResolve.depthResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
		_depthStencilResolve.stencilResolveMode = VK_RESOLVE_MODE_SAMPLE_ZERO_BIT;
		_depthStencilResolve.pDepthStencilResolveAttachment = &_depthStencilResolveReference;
		
		subpass.pNext = &_depthStencilResolve;
	}

	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = nullptr;
	_subpasses.emplace_back(subpass);

	VkSubpassDependency2 dependency{};
	dependency.sType = VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
	dependency.pNext = nullptr;
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
		| VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT
		| VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	dependency.dependencyFlags = 0;

	_dependencies.emplace_back(dependency);
}

void VKRenderPass::CreateRenderPass()
{
	VK_STRUCT_CREATE(VkRenderPassCreateInfo2, renderPassCreateInfo, VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2);
	renderPassCreateInfo.attachmentCount = _descriptions.size();
	renderPassCreateInfo.pAttachments = _descriptions.data();
	renderPassCreateInfo.subpassCount = _subpasses.size();
	renderPassCreateInfo.pSubpasses = _subpasses.data();
	renderPassCreateInfo.dependencyCount = _dependencies.size();
	renderPassCreateInfo.pDependencies = _dependencies.data();

	VK_RESULT_CHECK(vkCreateRenderPass2, Device(), &renderPassCreateInfo, nullptr, &_renderPass);
}



MORISA_NAMESPACE_END


