#include "VKMesh.h"
#include "VKBuffer.h"
#include "VKContext.h"
#include "VKImage.h"
#include "Runtime/Graphics/MaterialData.h"

MORISA_NAMESPACE_BEGIN

VKMesh::VKMesh(MMesh* mMesh)
{
	_meshDatas.resize(1);
	FillMeshData(_meshDatas[0], mMesh);
	SetDescription();
}

VKMesh::VKMesh(const char* path)
{
	FillData(path);
	SetDescription();
}

VKMesh::~VKMesh()
{
	VKImageManager* imageManager = Context()->ImageManager();
	
	// Model images only for unique usage.
	for (MMap<MString, MVector<VKImage*>>::iterator it = _images.begin(); it != _images.end(); ++it)
	{
		for (VKImage* image : it->second)
		{
			imageManager->PushDelayDestroy(image);
		}
	}
}

void VKMesh::SetDescription()
{
	_bindingDescriptions.resize(1);

	_bindingDescriptions[0].binding = 0;
	_bindingDescriptions[0].stride = sizeof(MMeshVertexData);
	_bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_attributeDescriptions.resize(4);

	_attributeDescriptions[0].binding = 0;
	_attributeDescriptions[0].location = 0;
	_attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	_attributeDescriptions[0].offset = offsetof(MMeshVertexData, position);

	_attributeDescriptions[1].binding = 0;
	_attributeDescriptions[1].location = 1;
	_attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	_attributeDescriptions[1].offset = offsetof(MMeshVertexData, normal);

	_attributeDescriptions[2].binding = 0;
	_attributeDescriptions[2].location = 2;
	_attributeDescriptions[2].format = VK_FORMAT_R32G32B32_SFLOAT;
	_attributeDescriptions[2].offset = offsetof(MMeshVertexData, color);

	_attributeDescriptions[3].binding = 0;
	_attributeDescriptions[3].location = 3;
	_attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
	_attributeDescriptions[3].offset = offsetof(MMeshVertexData, uv);
}

void VKMesh::FillMeshData(VKMeshData& meshData, MMesh* mMesh)
{
	VKBufferManager* manager = Context()->BufferManager();

	meshData.drawType = kMDrawTypeDrawIndex;
	meshData.vertexData = mMesh->vertices.data();
	meshData.vertexCount = mMesh->vertices.size();
	meshData.vertexSize = ((VkDeviceSize)mMesh->vertices.size()) * sizeof(MMeshVertexData);

	meshData.indexData = mMesh->indices.data();
	meshData.indexCount = mMesh->indices.size();
	meshData.indexSize = ((VkDeviceSize)mMesh->indices.size()) * sizeof(uint32_t);

	meshData.vertexBuffer = manager->CreatePersistenceBuffer(
		meshData.vertexSize,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		true,
		meshData.vertexData);

	meshData.indexBuffer = manager->CreatePersistenceBuffer(
		meshData.indexSize,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		true,
		meshData.indexData);

	if (mMesh->imageIndices.size() > 0)
	{
		meshData.imageIndices = std::move(mMesh->imageIndices);
	}
}

void VKMesh::FillData(const char* path)
{
	MModel* model = LoadModel(path);
	VKBufferManager* bufferManager = Context()->BufferManager();
	VKImageManager* imageManager = Context()->ImageManager();
	
	const MVector<MMesh*>& mMeshs = model->Meshs();
	const MVector<MImageData>& mImageDatas = model->ImageDatas();
	const uint32_t meshCount = mMeshs.size();
	const uint32_t imageCount = mImageDatas.size();
	_meshDatas.resize(meshCount);
	MVector<VKImage*> mainTexs(imageCount);

	for (uint32_t i = 0; i < meshCount; ++i)
	{
		FillMeshData(_meshDatas[i], mMeshs[i]);
	}

	for (uint32_t i = 0; i < imageCount; ++i)
	{
		mainTexs[i] = imageManager->CreateImageFormImage(mImageDatas[i].image);
	}

	_images.emplace(std::make_pair(MainTex, mainTexs));

	MORISA_DELETE(model);
}

VKMeshManager::VKMeshManager()
{
	_cacheDefaultMeshs.emplace(std::make_pair(kMDefaultMeshPlane, GenerateMeshDefault(&mPlaneMesh)));
	_cacheDefaultMeshs.emplace(std::make_pair(kMDefaultMeshQuad, GenerateMeshDefault(&mQuadMesh)));
	_cacheDefaultMeshs.emplace(std::make_pair(kMDefaultMeshCube, GenerateMeshDefault(&mCubeMesh)));

	assert(_cacheDefaultMeshs.size() == kMDefaultMeshCount);
}

VKMesh* VKMeshManager::CreateMeshDefault(MDefaultMesh defaultMesh)
{
	MUMap<MDefaultMesh, VKMesh*>::iterator it = _cacheDefaultMeshs.find(defaultMesh);
	return it->second;
}

VKMesh* VKMeshManager::CreateMeshModel(const char* path)
{
	MUMap<MString, VKMesh*>::iterator it = _cacheModelMeshs.find(path);
	if (it != _cacheModelMeshs.cend())
	{
		return it->second;
	}
	VKMesh* mesh = MORISA_NEW(VKMesh, path);
	PushPersistence(mesh);
	_cacheModelMeshs.emplace(std::make_pair(path, mesh));
	return mesh;
}

VKMesh* VKMeshManager::GenerateMeshDefault(MMesh* mMesh)
{
	VKMesh* mesh = MORISA_NEW(VKMesh, mMesh);
	PushPersistence(mesh);
	return mesh;
}

MORISA_NAMESPACE_END
