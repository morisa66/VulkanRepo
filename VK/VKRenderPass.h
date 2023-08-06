#ifndef __VK_RENDER_PASS__
#define __VK_RENDER_PASS__

#include "VKInclude.h"
#include <vector>
#include <array>
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
	std::vector<uint8_t> colorIndices;
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
	std::vector<Attachment> attachments;
	std::vector<Subpass> subpasses;
};

class VKRenderPass
{
public:
	VKRenderPass();
	~VKRenderPass();
	inline VKRenderPassInfo& GetInfo() { return _info; }
	void Create();
	inline const VkRenderPass RenderPass() { return _renderPass; }
	void SetClearValue(const VkClearValue& clearColor, const VkClearValue& clearDepth);
	const std::array<VkClearValue, 2>& ClearValues() { return _clearValues; }
	VKFramebuffer* FrameBuffer() { return _frameBuffer; }
	void FlushRenderImages(std::initializer_list<VKImage*> images);
private:
	void FillAttachments();
	void CreateRenderPass();
private:
	VKRenderPassInfo _info;
	VkRenderPass _renderPass;
	std::vector<VkAttachmentDescription2> _descriptions;
	std::vector<VkAttachmentReference2> _colorReferences;
	VkAttachmentReference2 _depthReference;
	VkAttachmentReference2 _resolveReference;
	VkAttachmentReference2 _depthStencilResolveReference;
	VkSubpassDescriptionDepthStencilResolve _depthStencilResolve;
	std::vector<VkSubpassDescription2> _subpasses;
	std::vector<VkSubpassDependency2> _dependencies;
	std::array<VkClearValue, 2> _clearValues;

	VKFramebuffer* _frameBuffer;
};

MORISA_NAMESPACE_END

#endif
