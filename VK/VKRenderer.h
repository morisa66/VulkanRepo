#include "VKInclude.h"
#include "Runtime/Graphics/RenderNode.h"
#include "Runtime/Renderer/ShadowPass.h"
#include "Runtime/Renderer/OpaquePass.h"
#include "Runtime/Renderer/PostProcessPass.h"


MORISA_NAMESPACE_BEGIN


struct VKSynchronizationInfo;
class EventProcessor;
struct EventProcessorData;
class Camera;


class VKRenderer
{
public:
	VKRenderer();
	~VKRenderer();
	void WindowResize();
	void Update();
	void Sync();
	void RenderFrame();
	void AfterRender();
private:
	uint32_t _currentIndex;
	const VKSynchronizationInfo* _syncInfo;

	ShadowPass* _shadowPass;
	OpaquePass* _opaquePass;
	PostProcessPass* _postProcessPass;

	Camera* _camera;
	EventProcessor* _eventProcessor;
	EventProcessorData* _eventProcessorData;
};


MORISA_NAMESPACE_END
