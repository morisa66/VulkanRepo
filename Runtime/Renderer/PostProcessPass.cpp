#include "PostProcessPass.h"
#include "VK/VKContext.h"
#include "VK/VKImage.h"
#include "VK/VKMesh.h"
#include "VK/VKUniform.h"
#include "VK/VKPipelineState.h"
#include "VK/VKDescriptor.h"
#include "VK/VKFrameBuffer.h"
#include "Runtime/Graphics/Material.h"

MORISA_NAMESPACE_BEGIN

PostProcessPass::PostProcessPass(const VkExtent2D& extent, 
	VKImage* colorImage, VKImage* depthImage, VKImage* shadowMap)
{
	_extent = extent;

	PrepareRenderNode(colorImage, depthImage, shadowMap);

	CreateNativePass();
}

void PostProcessPass::FlushImage(VKImage* colorImage, VKImage* depthImage)
{
	for (MRenderNode* node : _renderNodes)
	{
		node->material->SetImage(ColorTex, colorImage);
		node->material->SetImage(DepthTex, depthImage);
		Context()->DescriptorManager()->Update(node->uniform);
	}
}

void PostProcessPass::SetCurrentBuffer(uint32_t index)
{
	_nativePass->FrameBuffer()->SetCurrentBufferIndex(index);
}

void PostProcessPass::ResizeRenderTarget(const VkExtent2D& extent)
{
	MRenderPass::ResizeRenderTarget(extent);
	
	_nativePass->FlushRenderImages({ VKImage::SwapChainImage() });
}

void PostProcessPass::PrepareRenderNode(VKImage* colorImage, VKImage* depthImage, VKImage* shadowMap)
{
	VKImageManager* imageManager = Context()->ImageManager();
	VKMeshManager* meshManager = Context()->MeshManager();
	VKUniformManager* uniformManager = Context()->UniformManager();

	MShader* fullScreenShader = MORISA_NEW(MShader, "/GLSL/FullScreen.vert", "/GLSL/FullScreen.frag");

	MRenderNode* fullScreenNode = GenerateNode();

 	fullScreenNode->mesh = meshManager->CreateMeshDefault(kMDefaultMeshQuad);
	fullScreenNode->material = MORISA_NEW(MMaterial, fullScreenShader);
	fullScreenNode->material->Data(kMShaderStageFragment)->AddImage(ColorTex, colorImage);
	fullScreenNode->material->Data(kMShaderStageFragment)->AddImage(DepthTex, depthImage);
	fullScreenNode->material->Data(kMShaderStageFragment)->AddImage("BlendImage",
		Context()->ImageManager()->CreateImageFromPath("/Pictures/1.jpg"));
	fullScreenNode->uniform = uniformManager->CreateUniform(fullScreenNode->material);

	fullScreenNode->state = MORISA_NEW(VKPipelineState);
	fullScreenNode->state->ConfigureVertexInput(fullScreenNode->mesh);
	fullScreenNode->state->ConfigureLayout(fullScreenNode->uniform);
}

void PostProcessPass::CreateNativePass()
{
	_nativePass = MORISA_NEW(VKRenderPass);
	VKRenderPassInfo& renderPassInfo = _nativePass->Info();
	renderPassInfo.scope.offset = { 0, 0 };
	renderPassInfo.scope.extent =_extent;
	renderPassInfo.attachments.resize(1);
	renderPassInfo.attachments[0].renderImage = VKImage::SwapChainImage();
	renderPassInfo.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	renderPassInfo.attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassInfo.attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	renderPassInfo.subpasses.resize(1);
	renderPassInfo.subpasses[0].colorIndices.emplace_back(0);

	renderPassInfo.clearValues.resize(1);
	renderPassInfo.clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f } };

	_nativePass->Create();
}

MORISA_NAMESPACE_END


