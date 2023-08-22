#ifndef __OPAQUE_PASS_H__
#define __OPAQUE_PASS_H__

#include "MRenderPass.h"
#include "Runtime/Camera.h"

MORISA_NAMESPACE_BEGIN

class OpaquePass : public MRenderPass
{
public:
	OpaquePass(const VkExtent2D& extent, VKImage* shadowMap, Camera* shadowCamera);
	virtual ~OpaquePass();
	VKImage* ColorImage() { return _colorImage; }
	VKImage* DepthImage() { return _depthImage; }

	virtual void ResizeRenderTarget(const VkExtent2D& extent) override;
private:
	void CreateRenderTarget();
	void PrepareRenderNode(VKImage* shadowMap, Camera* shadowCamera);
	void CreateNativePass();
private:
	VKImage* _colorImage;
	VKImage* _depthImage;
	// MSAA
	VKImage* _colorSampleImage;
	VKImage* _depthSampleImage;
};

MORISA_NAMESPACE_END

#endif
