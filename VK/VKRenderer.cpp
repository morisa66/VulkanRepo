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

VKRenderer::VKRenderer():
	_currentIndex(0)
	, _syncInfo(nullptr)
	, _eventProcessor(nullptr)
	, _eventProcessorData(nullptr)
	, _renderPass(nullptr)
	, _finalRenderPass(nullptr)
	, _colorImage(nullptr)
	, _depthImage(nullptr)
	, _colorSampleImage(nullptr)
	, _depthSampleImage(nullptr)
{
	config.useMSAA = 1;

	VKContext* context = Context();

	CreateRenderTarget();

	_camera = MORISA_NEW(Camera, context->WindowSize().width, context->WindowSize().height, 45.0f, 0.1f, 100.0f);

	_eventProcessor = MORISA_NEW(EventProcessor);
	_eventProcessorData = MORISA_NEW(EventProcessorData, _camera);
	context->SetEventProcessorData(_eventProcessorData);

	_FullScreenNode = MORISA_NEW(MRenderNode);
	_FullScreenNode->mesh = context->MeshManager()->CreateMeshDefault(kMDefaultMeshFullScreen);
	_FullScreenNode->material = MORISA_NEW(MMaterial, MORISA_NEW(MShader, "/GLSL/FullScreen.vert", "/GLSL/FullScreen.frag"));
	_FullScreenNode->material->Data(kMShaderStageFragment)->AddImage("ColorImage", _colorImage);
	_FullScreenNode->material->Data(kMShaderStageFragment)->AddImage("DepthImage", _depthImage);
	_FullScreenNode->material->Data(kMShaderStageFragment)->AddImage("BlendImage",
		Context()->ImageManager()->CreateImageFromAsset("/Pictures/1.jpg"));
	_FullScreenNode->uniform = context->UniformManager()->CreateUniform(_FullScreenNode->material);

	_cubeNode = MORISA_NEW(MRenderNode);
	_cubeNode->mesh = context->MeshManager()->CreateMeshDefault(kMDefaultMeshCube);
	_cubeNode->material = MORISA_NEW(MMaterial, MORISA_NEW(MShader, "/GLSL/DefaultMesh.vert", "/GLSL/DefaultMesh.frag"));
	_cubeNode->material->Data(kMShaderStageVertex)->AddMat("Model", glm::scale(
		glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)), glm::vec3(0.2f, 0.2f, 0.2f)));
		
	_cubeNode->material->Data(kMShaderStageFragment)->AddImage("MainTex", 
		Context()->ImageManager()->CreateImageFromAsset("/Pictures/Kokomi.jpg"));
	_cubeNode->uniform = context->UniformManager()->CreateUniform(_cubeNode->material);

	_modelNode = MORISA_NEW(MRenderNode);
	_modelNode->mesh = context->MeshManager()->CreateMeshModel("/Models/mary/Marry.obj");
	_modelNode->material = MORISA_NEW(MMaterial, MORISA_NEW(MShader, "/GLSL/PhongLighting.vert", "/GLSL/PhongLighting.frag"));
	_modelNode->material->Data(kMShaderStageVertex)->AddMat("Model", glm::mat4(1.0f));
	_modelNode->material->Data(kMShaderStageFragment)->AddImage("MainTex", 
		Context()->ImageManager()->CreateImageFromAsset("/Models/mary/MC003_Kozakura_Mari.png"));
	_modelNode->uniform = context->UniformManager()->CreateUniform(_modelNode->material);

	_cubeNode->state = MORISA_NEW(VKPipelineState);
	_cubeNode->state->ConfigureVertexInput(_cubeNode->mesh);
	_cubeNode->state->ConfigureLayout(_cubeNode->uniform);

	_modelNode->state = MORISA_NEW(VKPipelineState);
	_modelNode->state->ConfigureVertexInput(_modelNode->mesh);
	_modelNode->state->ConfigureLayout(_modelNode->uniform);

	_FullScreenNode->state = MORISA_NEW(VKPipelineState);
	_FullScreenNode->state->ConfigureVertexInput(_FullScreenNode->mesh);
	_FullScreenNode->state->ConfigureLayout(_FullScreenNode->uniform);

	if (config.useMSAA)
	{
		_renderPass = MORISA_NEW(VKRenderPass);
		VKRenderPassInfo& renderPassInfo = _renderPass->GetInfo();
		renderPassInfo.attachments.resize(4);
		renderPassInfo.attachments[0].renderImage = _colorSampleImage;
		renderPassInfo.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		renderPassInfo.attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		renderPassInfo.attachments[1].renderImage = _depthSampleImage;
		renderPassInfo.attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		renderPassInfo.attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		renderPassInfo.attachments[2].renderImage = _colorImage;
		renderPassInfo.attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		renderPassInfo.attachments[3].renderImage = _depthImage;
		renderPassInfo.attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[3].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		renderPassInfo.subpasses.resize(1);
		renderPassInfo.subpasses[0].colorIndices.emplace_back(0);
		renderPassInfo.subpasses[0].depthIndex = 1;
		renderPassInfo.subpasses[0].resolveIndex = 2;
		renderPassInfo.subpasses[0].depthResolveIndex = 3;
		_renderPass->Create();

		_cubeNode->state->ConfigureSamples(_colorSampleImage->Info().samples);
		_modelNode->state->ConfigureSamples(_colorSampleImage->Info().samples);
	}
	else
	{
		_renderPass = MORISA_NEW(VKRenderPass);
		VKRenderPassInfo& renderPassInfo = _renderPass->GetInfo();
		renderPassInfo.attachments.resize(2);
		renderPassInfo.attachments[0].renderImage = _colorImage;
		renderPassInfo.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		renderPassInfo.attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		renderPassInfo.attachments[1].renderImage = _depthImage;
		renderPassInfo.attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		renderPassInfo.attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

		renderPassInfo.subpasses.resize(1);
		renderPassInfo.subpasses[0].colorIndices.emplace_back(0);
		renderPassInfo.subpasses[0].depthIndex = 1;

		_renderPass->Create();
	}

	{
		_finalRenderPass = MORISA_NEW(VKRenderPass);
		VKRenderPassInfo& renderPassInfo = _finalRenderPass->GetInfo();
		renderPassInfo.attachments.resize(1);
		renderPassInfo.attachments[0].renderImage = VKImage::SwapChainImage();
		renderPassInfo.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		renderPassInfo.attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		renderPassInfo.attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		renderPassInfo.subpasses.resize(1);
		renderPassInfo.subpasses[0].colorIndices.emplace_back(0);
		
		_finalRenderPass->Create();
	}
}

VKRenderer::~VKRenderer()
{
	MORISA_DELETE(_camera);
	MORISA_DELETE(_eventProcessor);
	MORISA_DELETE(_eventProcessorData);
	MORISA_DELETE(_cubeNode);
	MORISA_DELETE(_modelNode);
	MORISA_DELETE(_FullScreenNode);
	MORISA_DELETE(_renderPass);
	MORISA_DELETE(_finalRenderPass);
}

void VKRenderer::WindowResize()
{
	VKContext* context = Context();

	_camera->ResetExtent(context->WindowSize().width, context->WindowSize().height);

	CreateRenderTarget();

	if (config.useMSAA)
	{
		_renderPass->FlushRenderImages({ _colorSampleImage, _depthSampleImage, _colorImage, _depthImage });
	}
	else
	{
		_renderPass->FlushRenderImages({ _colorImage, _depthImage });
	}
	_finalRenderPass->FlushRenderImages({VKImage::SwapChainImage()});

	_FullScreenNode->material->SetImage("ColorImage", _colorImage);
	_FullScreenNode->material->SetImage("DepthImage", _depthImage);
	context->DescriptorManager()->Update(_FullScreenNode->uniform);
}

void VKRenderer::Update()
{
	VKContext* context = Context();
	_eventProcessor->Process(_eventProcessorData);

	_camera->Update();
	context->GlobalMaterial()->SetMat("view", _camera->View());
	context->GlobalMaterial()->SetMat("proj", _camera->Projection());
	Context()->UniformManager()->Update();
}

void VKRenderer::Sync()
{
	VKSynchronization* sync = Context()->Synchronization();
	sync->SyncLast();

	_syncInfo = sync->GetCurrentInfo();
	_currentIndex = Context()->GetSwapChainImageIndex(_syncInfo->imageAvailableSemaphore);
	_finalRenderPass->FrameBuffer()->SetCurrentBufferIndex(_currentIndex);
	Context()->CommandBuffer()->Recycle();
}

void VKRenderer::RenderFrame()
{
	VKContext* context = Context();

	VKCommandBuffer* commandBuffer = context->CommandBuffer();

	commandBuffer->Begin();
	
	VkRect2D renderArea = { {0, 0}, {Context()->WindowSize()} };
	VkViewport viewport{ 0.0f, 0.0f, (float)context->WindowSize().width,
	(float)context->WindowSize().height, 0.0f, 1.0f };

	commandBuffer->BeginRenderPass(_renderPass, renderArea);
	commandBuffer->SetViewport(viewport, renderArea);
	DrawNode(_cubeNode, _renderPass);
	DrawNode(_modelNode, _renderPass);
	commandBuffer->EndRenderPass();

	commandBuffer->BeginRenderPass(_finalRenderPass, renderArea);
	commandBuffer->SetViewport(viewport, renderArea);
	DrawNode(_FullScreenNode, _finalRenderPass);
	commandBuffer->EndRenderPass();

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

void VKRenderer::DrawNode(MRenderNode* node, VKRenderPass* pass)
{
	VKContext* context = Context();
	VKCommandBuffer* commandBuffer = context->CommandBuffer();

	VKPipelineKey pipelineKey;
	pipelineKey.program = node->material->Shader()->Program();
	pipelineKey.state = node->state;
	pipelineKey.pass = pass;
	VkPipeline pipeline = context->PipelineCache()->GetCachePipeline(pipelineKey);

	// TODO if Pipeline not changed, don't bind it twice
	commandBuffer->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
	commandBuffer->BindDescriptorSets(node->state, node->uniform);

	for (int i = 0; i < node->mesh->GetMeshCount(); ++i)
	{
		const VKMeshData& meshData = node->mesh->GetMeshData(i);
		commandBuffer->BindVertexBuffer(meshData.vertexBuffer);
		if (meshData.drawType == kMDrawTypeDraw)
		{
			commandBuffer->Draw(meshData.vertexCount, 1, 0, 0);
		}
		else
		{
			commandBuffer->BindIndexBuffer(meshData.indexBuffer, VK_INDEX_TYPE_UINT32);
			commandBuffer->DrawIndexed(meshData.indexCount, 1, 0, 0, 0);
		}
	}
}

void VKRenderer::CreateRenderTarget()
{
	VKContext* context = Context();
	VKImageManager* manager = context->ImageManager();

	manager->PushDelayDestroy(_colorImage);
	manager->PushDelayDestroy(_depthImage);

;	VKImageInfo colorInfo;
	colorInfo.format = context->GetSwapChainImageInfo().format;
	colorInfo.width = context->WindowSize().width;
	colorInfo.height = context->WindowSize().height;
	colorInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	colorInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	_colorImage = manager->CreateImage(&colorInfo);

	VKImageInfo depthInfo;
	depthInfo.format = VK_FORMAT_D32_SFLOAT;
	depthInfo.width = context->WindowSize().width;
	depthInfo.height = context->WindowSize().height;
	depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	depthInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthInfo.depthBit = 1;
	_depthImage = manager->CreateImage(&depthInfo);


	if (config.useMSAA)
	{
		manager->PushDelayDestroy(_colorSampleImage);
		manager->PushDelayDestroy(_depthSampleImage);

		VKImageInfo colorSampleInfo;
		colorSampleInfo.format = context->GetSwapChainImageInfo().format;
		colorSampleInfo.width = context->WindowSize().width;
		colorSampleInfo.height = context->WindowSize().height;
		colorSampleInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		colorSampleInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		colorSampleInfo.samples = manager->MsaaSamples();
		_colorSampleImage = manager->CreateImage(&colorSampleInfo);

		VKImageInfo depthSampleInfo;
		depthSampleInfo.format = VK_FORMAT_D32_SFLOAT;
		depthSampleInfo.width = context->WindowSize().width;
		depthSampleInfo.height = context->WindowSize().height;
		depthSampleInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		depthSampleInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthSampleInfo.depthBit = 1;
		depthSampleInfo.samples = manager->MsaaSamples();
		_depthSampleImage = manager->CreateImage(&depthSampleInfo);
	}
}


MORISA_NAMESPACE_END