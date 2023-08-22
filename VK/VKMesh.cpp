#include "VKMesh.h"
#include "VKBuffer.h"
#include "VKContext.h"

MORISA_NAMESPACE_BEGIN

struct MDefaultMeshVertexData
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

static MVector<glm::vec3> cubePositionData =
{
	{-0.5f,-0.5f,-0.5f,},  // -X side	
	{-0.5f,-0.5f, 0.5f,},
	{-0.5f, 0.5f, 0.5f,},
	{-0.5f, 0.5f, 0.5f,},
	{-0.5f, 0.5f,-0.5f,},
	{-0.5f,-0.5f,-0.5f,},

	{-0.5f,-0.5f,-0.5f,},  // -Z side
	{ 0.5f, 0.5f,-0.5f,},
	{ 0.5f,-0.5f,-0.5f,},
	{-0.5f,-0.5f,-0.5f,},
	{-0.5f, 0.5f,-0.5f,},
	{ 0.5f, 0.5f,-0.5f,},

	{-0.5f,-0.5f,-0.5f,},  // -Y side
	{ 0.5f,-0.5f,-0.5f,},
	{ 0.5f,-0.5f, 0.5f,},
	{-0.5f,-0.5f,-0.5f,},
	{ 0.5f,-0.5f, 0.5f,},
	{-0.5f,-0.5f, 0.5f,},

	{-0.5f, 0.5f,-0.5f,},  // +Y side
	{-0.5f, 0.5f, 0.5f,},
	{ 0.5f, 0.5f, 0.5f,},
	{-0.5f, 0.5f,-0.5f,},
	{ 0.5f, 0.5f, 0.5f,},
	{ 0.5f, 0.5f,-0.5f,},

	{ 0.5f, 0.5f,-0.5f,},  // +X side
	{ 0.5f, 0.5f, 0.5f,},
	{ 0.5f,-0.5f, 0.5f,},
	{ 0.5f,-0.5f, 0.5f,},
	{ 0.5f,-0.5f,-0.5f,},
	{ 0.5f, 0.5f,-0.5f,},

	{-0.5f, 0.5f, 0.5f,},  // +Z side
	{-0.5f,-0.5f, 0.5f,},
	{ 0.5f, 0.5f, 0.5f,},
	{-0.5f,-0.5f, 0.5f,},
	{ 0.5f,-0.5f, 0.5f,},
	{ 0.5f, 0.5f, 0.5f,},
};

static MVector<glm::vec2> cubeUVData =
{
	{0.0f, 0.0f,},  // -X side
	{1.0f, 0.0f,},
	{1.0f, 1.0f,},
	{1.0f, 1.0f,},
	{0.0f, 1.0f,},
	{0.0f, 0.0f,},

	{1.0f, 0.0f,},  // -Z side
	{0.0f, 1.0f,},
	{0.0f, 0.0f,},
	{1.0f, 0.0f,},
	{1.0f, 1.0f,},
	{0.0f, 1.0f,},

	{1.0f, 1.0f,},  // -Y side
	{1.0f, 0.0f,},
	{0.0f, 0.0f,},
	{1.0f, 1.0f,},
	{0.0f, 0.0f,},
	{0.0f, 1.0f,},

	{1.0f, 1.0f,},  // +Y side
	{0.0f, 1.0f,},
	{0.0f, 0.0f,},
	{1.0f, 1.0f,},
	{0.0f, 0.0f,},
	{1.0f, 0.0f,},

	{1.0f, 1.0f,},  // +X side
	{0.0f, 1.0f,},
	{0.0f, 0.0f,},
	{0.0f, 0.0f,},
	{1.0f, 0.0f,},
	{1.0f, 1.0f,},

	{0.0f, 1.0f,},  // +Z side
	{0.0f, 0.0f,},
	{1.0f, 1.0f,},
	{0.0f, 0.0f,},
	{1.0f, 0.0f,},
	{1.0f, 1.0f,},
};

static MVector<glm::vec3> fullScreenPositionData =
{
	{-1.0f, -1.0f, 0.0f,},
	{3.0f, -1.0f, 0.0f,},
	{-1.0f, 3.0f, 0.0f,},
};

static MVector<glm::vec2> fullScreenUVData =
{
	{0.0f, 0.0f},
	{2.0f, 0.0f},
	{0.0f, 2.0f},
};

VKMesh::VKMesh(const char* path)
{
	FillData(path);
	SetDescriptionModel();
}

VKMesh::VKMesh(MDefaultMesh defaultMesh)
{
	FillData(defaultMesh);
	SetDescriptionDefault();
}

VKMesh::~VKMesh()
{
}

void VKMesh::SetDescriptionModel()
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

void VKMesh::SetDescriptionDefault()
{
	_bindingDescriptions.resize(1);

	_bindingDescriptions[0].binding = 0;
	_bindingDescriptions[0].stride = sizeof(MDefaultMeshVertexData);
	_bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	_attributeDescriptions.resize(3);

	_attributeDescriptions[0].binding = 0;
	_attributeDescriptions[0].location = 0;
	_attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	_attributeDescriptions[0].offset = offsetof(MDefaultMeshVertexData, position);

	_attributeDescriptions[1].binding = 0;
	_attributeDescriptions[1].location = 1;
	_attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	_attributeDescriptions[1].offset = offsetof(MDefaultMeshVertexData, normal);

	_attributeDescriptions[2].binding = 0;
	_attributeDescriptions[2].location = 2;
	_attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	_attributeDescriptions[2].offset = offsetof(MDefaultMeshVertexData, uv);
}

void VKMesh::FillData(const char* path)
{
	MModel* model = LoadModel(path);
	VKBufferManager* bufferManager = Context()->BufferManager();
	
	const MVector<MMesh*> mMeshs = model->Meshs();
	const uint32_t count = mMeshs.size();
	_meshDatas.resize(count);

	for (uint32_t i = 0; i < count; ++i)
	{
		_meshDatas[i].drawType = kMDrawTypeDrawIndex;
		_meshDatas[i].vertexData = mMeshs[i]->vertices.data();
		_meshDatas[i].vertexCount = mMeshs[i]->vertices.size();
		_meshDatas[i].vertexSize = ((VkDeviceSize)mMeshs[i]->vertices.size()) * sizeof(MMeshVertexData);

		_meshDatas[i].indexData = mMeshs[i]->indices.data();
		_meshDatas[i].indexCount = mMeshs[i]->indices.size();
		_meshDatas[i].indexSize = ((VkDeviceSize)mMeshs[i]->indices.size()) * sizeof(uint32_t);

		_meshDatas[i].vertexBuffer = bufferManager->CreatePersistenceBuffer(
			_meshDatas[i].vertexSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			true,
			_meshDatas[i].vertexData);

		_meshDatas[i].indexBuffer = bufferManager->CreatePersistenceBuffer(
			_meshDatas[i].indexSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
			true,
			_meshDatas[i].indexData);
	}

	MORISA_DELETE(model);
}

void VKMesh::FillData(MDefaultMesh defaultMesh)
{
	VKBufferManager* bufferManager = Context()->BufferManager();
	_meshDatas.resize(1);

	switch (defaultMesh)
	{
	case kMDefaultMeshCube:
	{
		MVector<MDefaultMeshVertexData> cubeData(cubePositionData.size());
		for (uint32_t i = 0; i < cubePositionData.size(); ++i)
		{
			cubeData[i].position = cubePositionData[i];
			// Not normal !
			cubeData[i].normal = glm::vec3(0.0f);
			cubeData[i].uv = cubeUVData[i];
		}

		_meshDatas[0].drawType = kMDrawTypeDraw;
		_meshDatas[0].vertexData = cubeData.data();
		_meshDatas[0].vertexCount = cubeData.size();
		_meshDatas[0].vertexSize = ((VkDeviceSize)cubeData.size()) * sizeof(MDefaultMeshVertexData);

		_meshDatas[0].vertexBuffer = bufferManager->CreatePersistenceBuffer(
			_meshDatas[0].vertexSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			true,
			_meshDatas[0].vertexData);
		break;
	}
	case kMDefaultMeshFullScreen:
	{
		MVector<MDefaultMeshVertexData> fullScreenData(fullScreenPositionData.size());
		for (uint32_t i = 0; i < fullScreenPositionData.size(); ++i)
		{
			fullScreenData[i].position = fullScreenPositionData[i];
			// Not normal !
			fullScreenData[i].normal = glm::vec3(0.0f);
			fullScreenData[i].uv = fullScreenUVData[i];
		}

		_meshDatas[0].drawType = kMDrawTypeDraw;
		_meshDatas[0].vertexData = fullScreenData.data();
		_meshDatas[0].vertexCount = fullScreenData.size();
		_meshDatas[0].vertexSize = ((VkDeviceSize)fullScreenData.size()) * sizeof(MDefaultMeshVertexData);

		_meshDatas[0].vertexBuffer = bufferManager->CreatePersistenceBuffer(
			_meshDatas[0].vertexSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			true,
			_meshDatas[0].vertexData);
		break;
	}
	default:
		break;
	}
}

VKMesh* VKMeshManager::CreateMeshDefault(MDefaultMesh defaultMesh)
{
	VKMesh* mesh = MORISA_NEW(VKMesh, defaultMesh);
	PushPersistence(mesh);
	return mesh;
}

VKMesh* VKMeshManager::CreateMeshModel(const char* path)
{
	MUMap<std::string, VKMesh*>::const_iterator it = _cache.find(path);
	if (it != _cache.end())
	{
		return it->second;
	}
	VKMesh* mesh = MORISA_NEW(VKMesh, path);
	PushPersistence(mesh);
	return mesh;
}

MORISA_NAMESPACE_END
