#include "OpaquePass.h"
#include "VK/VKContext.h"
#include "VK/VKImage.h"
#include "VK/VKMesh.h"
#include "VK/VKUniform.h"
#include "VK/VKPipelineState.h"
#include "Runtime/Graphics/Material.h"

MORISA_NAMESPACE_BEGIN

OpaquePass::OpaquePass(const VkExtent2D& extent, VKImage* shadowMap, Camera* shadowCamera):
	_colorImage(nullptr)
	, _depthImage(nullptr)
	, _colorSampleImage(nullptr)
	, _depthSampleImage(nullptr)
{
	_extent = extent;

	CreateRenderTarget();

	PrepareRenderNode(shadowMap, shadowCamera);

	CreateNativePass();
}

OpaquePass::~OpaquePass()
{
	
}

void OpaquePass::ResizeRenderTarget(const VkExtent2D& extent)
{
	MRenderPass::ResizeRenderTarget(extent);

	CreateRenderTarget();

	if (globalConfig.useMSAA)
	{
		_nativePass->FlushRenderImages({ _colorSampleImage, _depthSampleImage, _colorImage, _depthImage });
	}
	else
	{
		_nativePass->FlushRenderImages({_colorImage, _depthImage });
	}
}

void OpaquePass::CreateRenderTarget()
{
	VKContext* context = Context();
	VKImageManager* manager = context->ImageManager();

	manager->PushDelayDestroy(_colorImage);
	manager->PushDelayDestroy(_depthImage);

	VKImageInfo colorInfo;
	colorInfo.format = context->GetSwapChainImageInfo().format;
	colorInfo.width = _extent.width;
	colorInfo.height = _extent.height;
	colorInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	colorInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	_colorImage = manager->CreateImage(&colorInfo);

	VKImageInfo depthInfo;
	depthInfo.format = VK_FORMAT_D32_SFLOAT;
	depthInfo.width = _extent.width;
	depthInfo.height = _extent.height;
	depthInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	depthInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthInfo.depthBit = 1;
	_depthImage = manager->CreateImage(&depthInfo);


	if (globalConfig.useMSAA)
	{
		manager->PushDelayDestroy(_colorSampleImage);
		manager->PushDelayDestroy(_depthSampleImage);

		VKImageInfo colorSampleInfo;
		colorSampleInfo.format = context->GetSwapChainImageInfo().format;
		colorSampleInfo.width = _extent.width;
		colorSampleInfo.height = _extent.height;
		colorSampleInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		colorSampleInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		colorSampleInfo.samples = manager->MsaaSamples();
		_colorSampleImage = manager->CreateImage(&colorSampleInfo);

		VKImageInfo depthSampleInfo;
		depthSampleInfo.format = VK_FORMAT_D32_SFLOAT;
		depthSampleInfo.width = _extent.width;
		depthSampleInfo.height = _extent.height;
		depthSampleInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
		depthSampleInfo.aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		depthSampleInfo.depthBit = 1;
		depthSampleInfo.samples = manager->MsaaSamples();
		_depthSampleImage = manager->CreateImage(&depthSampleInfo);
	}
}

void OpaquePass::PrepareRenderNode(VKImage* shadowMap, Camera* shadowCamera)
{
	VKImageManager* imageManager = Context()->ImageManager();
	VKMeshManager* meshManager = Context()->MeshManager();
	VKUniformManager* uniformManager = Context()->UniformManager();

	MShader* cubeShader = MORISA_NEW(MShader, "/GLSL/DefaultMesh.vert", "/GLSL/DefaultMesh.frag");
	MShader* lightingShader = MORISA_NEW(MShader, "/GLSL/PhongLighting.vert", "/GLSL/PhongLighting.frag");

	MRenderNode* cubeNode = GenerateNode();
	glm::vec4 shadowParmas = glm::vec4(globalConfig.shadowMapWidth, globalConfig.shadowMapHeight, 10.0f, 1000.0f);

	glm::vec3 LightPosition = Context()->GlobalMaterial()->Data(kMShaderStageFragment)->GetVec("LightPosition").xyz;
	cubeNode->mesh = meshManager->CreateMeshDefault(kMDefaultMeshCube);
	cubeNode->material = MORISA_NEW(MMaterial, cubeShader);
	cubeNode->material->Data(kMShaderStageVertex)->AddMat("Model", glm::scale(
			glm::translate(glm::mat4(1.0f), LightPosition),
			glm::vec3(0.2f, 0.2f, 0.2f)));
	cubeNode->material->Data(kMShaderStageFragment)->AddImage("MainTex",
		Context()->ImageManager()->CreateImageFromPath("/Pictures/Kokomi.jpg"));

	cubeNode->uniform = uniformManager->CreateUniform(cubeNode->material);
	cubeNode->state = MORISA_NEW(VKPipelineState);
	cubeNode->state->ConfigureVertexInput(cubeNode->mesh);
	cubeNode->state->ConfigureLayout(cubeNode->uniform);


	MRenderNode* modelNode = GenerateNode();

	//modelNode->mesh = meshManager->CreateMeshModel("/Models/mary/Marry.obj");
	modelNode->mesh = meshManager->CreateMeshModel("/Models/Kelala/Kelala.pmx");
	modelNode->material = MORISA_NEW(MMaterial, lightingShader);
	modelNode->material->Data(kMShaderStageVertex)->AddMat("Model", 
		glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)));
	modelNode->material->Data(kMShaderStageVertex)->AddMat("ShadowVP", 
		shadowCamera->Projection() * shadowCamera->View());
	modelNode->material->Data(kMShaderStageFragment)->AddVec("shadowParams", shadowParmas);
	modelNode->material->Data(kMShaderStageFragment)->AddImage(MainTex, VKImage::White);
	modelNode->material->Data(kMShaderStageFragment)->AddImage("ShadowMap", shadowMap);

	modelNode->uniform = uniformManager->CreateUniform(modelNode->material, modelNode->mesh->GetImages());

	modelNode->state = MORISA_NEW(VKPipelineState);
	modelNode->state->ConfigureVertexInput(modelNode->mesh);
	modelNode->state->ConfigureLayout(modelNode->uniform);

	modelNode->info.needNodeImage = true;

	MRenderNode* planeNode = GenerateNode();

	planeNode->mesh = meshManager->CreateMeshDefault(kMDefaultMeshPlane);
	planeNode->material = MORISA_NEW(MMaterial, lightingShader);
	planeNode->material->Data(kMShaderStageVertex)->AddMat("Model", 
		glm::scale(glm::mat4(1.0f), glm::vec3(4.0f, 1.0f, 4.0f)));
	planeNode->material->Data(kMShaderStageVertex)->AddMat("ShadowVP",
		shadowCamera->Projection() * shadowCamera->View());
	planeNode->material->Data(kMShaderStageFragment)->AddVec("shadowParams", shadowParmas);
	planeNode->material->Data(kMShaderStageFragment)->AddImage(MainTex,
		Context()->ImageManager()->CreateImageFromPath("/Pictures/2.jpg"));
	planeNode->material->Data(kMShaderStageFragment)->AddImage("ShadowMap", shadowMap);

	planeNode->uniform = uniformManager->CreateUniform(planeNode->material);
	planeNode->state = MORISA_NEW(VKPipelineState);
	planeNode->state->ConfigureVertexInput(planeNode->mesh);
	planeNode->state->ConfigureLayout(planeNode->uniform);
}

void OpaquePass::CreateNativePass()
{
	_nativePass = MORISA_NEW(VKRenderPass);
	VKRenderPassInfo& renderPassInfo = _nativePass->Info();
	renderPassInfo.scope.offset = { 0, 0 };
	renderPassInfo.scope.extent = _extent;

	if (globalConfig.useMSAA)
	{
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

		renderPassInfo.clearValues.resize(2);
		renderPassInfo.clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f } };
		renderPassInfo.clearValues[1].depthStencil = { 1.0f, 0 };

		_nativePass->Create();

		for (MRenderNode* node : _renderNodes)
		{
			node->state->ConfigureSamples(_colorSampleImage->Info().samples);
		}
	}
	else
	{
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

		renderPassInfo.clearValues.resize(2);
		renderPassInfo.clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f } };
		renderPassInfo.clearValues[1].depthStencil = { 1.0f, 0 };

		_nativePass->Create();
	}
}

MORISA_NAMESPACE_END