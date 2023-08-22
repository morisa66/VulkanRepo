#ifndef __VK_RENDER_PASS__
#define __VK_RENDER_PASS__

#include "VKInclude.h"
#include "Core/MVector.h"
#include <initializer_list>

MORISA_NAMESPACE_BEGIN

class VKImage;
class VKFramebuffer;

struct Attachment
{
	// renderImage == nullptr use swapchian image
	VKImage* renderImage;
	VkAttachmentLoadOp loadOp;
	VkAttachmentStoreOp storeOp;
	VkAttachmentLoadOp stencilloadOp;
	VkAttachmentStoreOp stencilStoreOp;
	VkImageLayout finalLayout;

	Attachment() :
		renderImage(nullptr)
		, loadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
		, storeOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
		, stencilloadOp(VK_ATTACHMENT_LOAD_OP_DONT_CARE)
		, stencilStoreOp(VK_ATTACHMENT_STORE_OP_DONT_CARE)
		, finalLayout(VK_IMAGE_LAYOUT_UNDEFINED)
	{
	}

	VkFormat Format() const;
	VkSampleCountFlagBits Samples() const;
	VkImageView View() const;
	VkImageAspectFlags Aspect() const;
};

struct Subpass
{
	static const uint8_t INVALID_INDEX = 0xFF;
	MVector<uint8_t> colorIndices;
	uint8_t depthIndex;
	uint8_t resolveIndex;
	uint8_t depthResolveIndex;

	Subpass():
		depthIndex(INVALID_INDEX)
		, resolveIndex(INVALID_INDEX)
		, depthResolveIndex(INVALID_INDEX)
	{
	}
};

// only support single pass
struct VKRenderPassInfo
{
	VkRect2D scope;
	MVector<Attachment> attachments;
	MVector<Subpass> subpasses;
	MVector<VkClearValue> clearValues;
};

class VKRenderPass
{
public:
	VKRenderPass();
	~VKRenderPass();
	inline VKRenderPassInfo& Info() { return _info; }
	void Create();
	inline const VkRenderPass RenderPass() { return _renderPass; }
	VKFramebuffer* FrameBuffer() { return _frameBuffer; }
	void FlushRenderImages(std::initializer_list<VKImage*> images);
private:
	void FillAttachments();
	void CreateRenderPass();
private:
	VKRenderPassInfo _info;
	VkRenderPass _renderPass;
	MVector<VkAttachmentDescription2> _descriptions;
	MVector<VkAttachmentReference2> _colorReferences;
	VkAttachmentReference2 _depthReference;
	VkAttachmentReference2 _resolveReference;
	VkAttachmentReference2 _depthStencilResolveReference;
	VkSubpassDescriptionDepthStencilResolve _depthStencilResolve;
	MVector<VkSubpassDescription2> _subpasses;
	MVector<VkSubpassDependency2> _dependencies;
	VKFramebuffer* _frameBuffer;
};

MORISA_NAMESPACE_END

#endif
