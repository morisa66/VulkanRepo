#ifndef __POST_PROCESS_PASS_H__
#define __POST_PROCESS_PASS_H__

#include "MRenderPass.h"

MORISA_NAMESPACE_BEGIN

class PostProcessPass : public MRenderPass
{
public:
	PostProcessPass(const VkExtent2D& extent, VKImage* colorImage, 
		VKImage* depthImage, VKImage* shadowMap);
	void FlushImage(VKImage* colorImage, VKImage* depthImage);
	void SetCurrentBuffer(uint32_t index);

	virtual void ResizeRenderTarget(const VkExtent2D& extent) override;
private:
	void PrepareRenderNode(VKImage* colorImage, VKImage* depthImage, VKImage* shadowMap);
	void CreateNativePass();
};

MORISA_NAMESPACE_END

#endif