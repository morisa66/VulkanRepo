#include "VKInclude.h"
#include "Runtime/Graphics/RenderNode.h"

MORISA_NAMESPACE_BEGIN

class VKPipelineState;
class VKGpuProgram;
class VKRenderPass;
struct VKSynchronizationInfo;
class EventProcessor;
struct EventProcessorData;
class MMaterial;
class VKMesh;
class Camera;
class VKImage;

struct RenderConfig
{
	uint32_t useMSAA : 1;

	RenderConfig():
		useMSAA(0)
	{

	}
};

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
	void DrawNode(MRenderNode* node, VKRenderPass* pass);
	void CreateRenderTarget();
private:
	RenderConfig config;

	uint32_t _currentIndex;
	const VKSynchronizationInfo* _syncInfo;

	VKRenderPass* _renderPass;
	VKRenderPass* _finalRenderPass;

	VKImage* _colorImage;
	VKImage* _depthImage;
	VKImage* _colorSampleImage;
	VKImage* _depthSampleImage;

	Camera* _camera;
	EventProcessor* _eventProcessor;
	EventProcessorData* _eventProcessorData;

	MRenderNode* _cubeNode;
	MRenderNode* _modelNode;
	MRenderNode* _FullScreenNode;
};


MORISA_NAMESPACE_END
