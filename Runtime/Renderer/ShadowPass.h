#ifndef __SHADOW_PASS_H__
#define __SHADOW_PASS_H__

#include "MRenderPass.h"
#include "Runtime/Camera.h"

MORISA_NAMESPACE_BEGIN

class ShadowPass : public MRenderPass
{
public:
	 ShadowPass();
	 virtual ~ShadowPass();
	 Camera* ShadowCamera() { return _shadowCamera; }
	 VKImage* ShadowMap() { return _shadowMap; }
private:
	void CreateRenderTarget();
	void PrepareRenderNode();
	void CreateNativePass();
private:
	Camera* _shadowCamera;
	VKImage* _shadowMap;
};

MORISA_NAMESPACE_END

#endif
