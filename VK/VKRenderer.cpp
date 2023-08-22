#include "VKRenderer.h"
#include "VKContext.h"
#include "VKPipelineState.h"
#include "VKPipeline.h"
#include "VKCommandBuffer.h"
#include "VKGpuProgram.h"
#include "VKRenderPass.h"
#include "VKFrameBuffer.h"
#include "VKSynchronization.h"
#include "VKBuffer.h"
#include "VKImage.h"
#include "VKMesh.h"
#include "VKUniform.h"
#include "VKDescriptor.h"
#include "Runtime/EventProcessor.h"
#include "Runtime/Graphics/Material.h"

MORISA_NAMESPACE_BEGIN

// Depth bias (and slope) are used to avoid shadowing artifacts
// Constant depth bias factor (always applied)
float depthBiasConstant = 1.25f;
// Slope depth bias factor, applied depending on polygon's slope
float depthBiasSlope = 1.75f;

VKRenderer::VKRenderer():
	_currentIndex(0)
	, _syncInfo(nullptr)
	, _eventProcessor(nullptr)
	, _eventProcessorData(nullptr)
	, _shadowPass(nullptr)
	, _opaquePass(nullptr)
	, _postProcessPass(nullptr)
{
	VKContext* context = Context();

	_camera = MORISA_NEW(Camera, context->WindowSize().width, context->WindowSize().height, 45.0f, 0.1f, 100.0f);

	_eventProcessor = MORISA_NEW(EventProcessor);
	_eventProcessorData = MORISA_NEW(EventProcessorData, _camera);

	context->SetEventProcessorData(_eventProcessorData);
	
	_shadowPass = MORISA_NEW(ShadowPass);

	_opaquePass = MORISA_NEW(OpaquePass, 
		context->WindowSize(), 
		_shadowPass->ShadowMap(), 
		_shadowPass->ShadowCamera());

	_postProcessPass = MORISA_NEW(PostProcessPass,
		context->WindowSize(),
		_opaquePass->ColorImage(),
		_opaquePass->DepthImage(),
		_shadowPass->ShadowMap());

}

VKRenderer::~VKRenderer()
{
	MORISA_DELETE(_camera);

	MORISA_DELETE(_eventProcessor);
	MORISA_DELETE(_eventProcessorData);

	MORISA_DELETE(_shadowPass);
	MORISA_DELETE(_opaquePass);
	MORISA_DELETE(_postProcessPass);
}

void VKRenderer::WindowResize()
{
	VKContext* context = Context();

	_camera->ResetExtent(context->WindowSize().width, context->WindowSize().height);
	

	_opaquePass->ResizeRenderTarget(context->WindowSize());

	_postProcessPass->ResizeRenderTarget(context->WindowSize());

	_postProcessPass->FlushImage(_opaquePass->ColorImage(), _opaquePass->DepthImage());
}

void VKRenderer::Update()
{
	VKContext* context = Context();
	_eventProcessor->Process(_eventProcessorData);

	_camera->Update();
	context->GlobalMaterial()->SetMat("View", _camera->View());
	context->GlobalMaterial()->SetMat("Proj", _camera->Projection());
	Context()->UniformManager()->Update();
}

void VKRenderer::Sync()
{
	VKSynchronization* sync = Context()->Synchronization();
	sync->SyncLast();

	_syncInfo = sync->GetCurrentInfo();
	_currentIndex = Context()->GetSwapChainImageIndex(_syncInfo->imageAvailableSemaphore);
	_postProcessPass->SetCurrentBuffer(_currentIndex);
	Context()->CommandBuffer()->Recycle();
}

void VKRenderer::RenderFrame()
{
	VKContext* context = Context();

	VKCommandBuffer* commandBuffer = context->CommandBuffer();

	commandBuffer->Begin();

	_shadowPass->Execute();
	_opaquePass->Execute();
	_postProcessPass->Execute();

	commandBuffer->End();
	commandBuffer->Submit(_syncInfo->fence, _syncInfo->imageAvailableSemaphore,
		_syncInfo->renderFinishedSemaphore,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
}

void VKRenderer::AfterRender()
{
	Context()->Present(_syncInfo->renderFinishedSemaphore);
	Context()->Synchronization()->Flush();
}



MORISA_NAMESPACE_END