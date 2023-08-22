#ifndef  __M_RENDER_PASS_H__
#define	 __M_RENDER_PASS_H__

#include "VK/VKRenderPass.h"
#include "Runtime/Graphics/RenderNode.h"
#include "Core/MVector.h"
#include "VK/VKInclude.h"

MORISA_NAMESPACE_BEGIN

class VKImage;

class MRenderPass
{
public:
	virtual ~MRenderPass();
	virtual void Execute();
	virtual void ResizeRenderTarget(const VkExtent2D& extent);
protected:
	MRenderPass();
	void RenderNode(MRenderNode* node);
	MRenderNode* GenerateNode();


protected:
	VkExtent2D _extent;
	VKRenderPass* _nativePass;
	MVector<MRenderNode*> _renderNodes;
};

MORISA_NAMESPACE_END

#endif 

