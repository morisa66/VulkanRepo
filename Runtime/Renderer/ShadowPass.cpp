#include "ShadowPass.h"

#include "VK/VKContext.h"
#include "VK/VKImage.h"
#include "VK/VKMesh.h"
#include "VK/VKUniform.h"
#include "VK/VKPipelineState.h"
#include "Runtime/Graphics/Material.h"

MORISA_NAMESPACE_BEGIN

ShadowPass::ShadowPass()
{
	_extent = { (uint32_t)globalConfig.shadowMapWidth , (uint32_t)globalConfig.shadowMapHeight };
	_shadowCamera = MORISA_NEW(Camera, _extent.width, _extent.height, 90.0f, 0.1f, 100.0f);
	_shadowCamera->MoveTo(Context()->GlobalMaterial()->
		Data(kMShaderStageFragment)->GetVec("LightPosition").xyz);
	_shadowCamera->LookAt(glm::vec3(0.0f));
	_shadowCamera->Update();

	CreateRenderTarget();

	PrepareRenderNode();

	CreateNativePass();
}

ShadowPass::~ShadowPass()
{
	MORISA_DELETE(_shadowCamera);
}

void ShadowPass::CreateRenderTarget()
{
	VKImageManager* imageManager = Context()->ImageManager();

	VKImageInfo shadowInfo;
	shadowInfo.format = VK_FORMAT_D32_SFLOAT;
	shadowInfo.width = _extent.width;
	shadowInfo.height = _extent.height;
	shadowInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	shadowInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	shadowInfo.depthBit = 1;
	_shadowMap = imageManager->CreateImage(&shadowInfo);
}

void ShadowPass::PrepareRenderNode()
{
	const float depthBiasConstant = 1.25f;
	const float depthBiasSlope = 1.75f;

	VKMeshManager* meshManager = Context()->MeshManager();
	VKUniformManager* uniformManager = Context()->UniformManager();

	MShader* shadowShader = MORISA_NEW(MShader, "/GLSL/Shadow.vert", "/GLSL/Shadow.frag");

	MRenderNode* shadowModelNode = GenerateNode();

	shadowModelNode->mesh = meshManager->CreateMeshModel("/Models/mary/Marry.obj");
	shadowModelNode->material = MORISA_NEW(MMaterial, shadowShader);
	shadowModelNode->material->Data(kMShaderStageVertex)->AddMat("Model", 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.2f, 0.2f, 0.2f)));
	shadowModelNode->material->Data(kMShaderStageVertex)->AddMat("ShadowVP",
		_shadowCamera->Projection() * _shadowCamera->View());
	shadowModelNode->uniform = uniformManager->CreateUniform(shadowModelNode->material);

	shadowModelNode->state = MORISA_NEW(VKPipelineState);
	shadowModelNode->state->ConfigureVertexInput(shadowModelNode->mesh);
	shadowModelNode->state->ConfigureLayout(shadowModelNode->uniform);

	MRenderNode* shadowPlaneNode = GenerateNode();

	shadowPlaneNode->mesh = meshManager->CreateMeshModel("/Models/Box.obj");
	shadowPlaneNode->material = MORISA_NEW(MMaterial, shadowShader);
	shadowPlaneNode->material->Data(kMShaderStageVertex)->AddMat("Model",
		glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 0.2f, 4.0f)));
	shadowPlaneNode->material->Data(kMShaderStageVertex)->AddMat("ShadowVP",
		_shadowCamera->Projection() * _shadowCamera->View());
	shadowPlaneNode->uniform = uniformManager->CreateUniform(shadowPlaneNode->material);

	shadowPlaneNode->state = MORISA_NEW(VKPipelineState);
	shadowPlaneNode->state->ConfigureVertexInput(shadowModelNode->mesh);
	shadowPlaneNode->state->ConfigureLayout(shadowModelNode->uniform);
}

void ShadowPass::CreateNativePass()
{
	_nativePass = MORISA_NEW(VKRenderPass);
	VKRenderPassInfo& renderPassInfo = _nativePass->Info();
	renderPassInfo.scope.offset = { 0, 0 };
	renderPassInfo.scope.extent = { _extent.width, _extent.height };
	renderPassInfo.attachments.resize(1);
	renderPassInfo.attachments[0].renderImage = _shadowMap;
	renderPassInfo.attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	renderPassInfo.attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	renderPassInfo.attachments[0].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

	renderPassInfo.subpasses.resize(1);
	renderPassInfo.subpasses[0].depthIndex = 0;

	renderPassInfo.clearValues.resize(1);
	renderPassInfo.clearValues[0].depthStencil = { 1.0f, 0 };

	_nativePass->Create();
}


MORISA_NAMESPACE_END

