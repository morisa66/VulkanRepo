#ifndef __VK_MESH_H__
#define __VK_MESH_H__

#include "VKInclude.h"
#include "Utils/MatrixUtils.h"
#include "Utils/ModelUtils.h"
#include "Core/GC.h"

#include"Core/MVector.h"

#include <string>

MORISA_NAMESPACE_BEGIN

class VKBuffer;

enum MDefaultMesh
{
	kMDefaultMeshCube,
	kMDefaultMeshFullScreen,
};

enum MDrawType
{
	kMDrawTypeDraw,
	kMDrawTypeDrawIndex,
};


struct VKMeshData
{
	MDrawType drawType;
	void* vertexData;
	void* indexData;
	uint32_t vertexCount;
	uint32_t indexCount;
	VkDeviceSize vertexSize;
	VkDeviceSize indexSize;
	VKBuffer* vertexBuffer;
	VKBuffer* indexBuffer;
};

class VKMeshManager;

class VKMesh : public GCNode
{
	friend class VKMeshManager;
public:
	virtual ~VKMesh();
	inline const MVector<VkVertexInputBindingDescription>& GetBindingDescriptions()
	{
		return _bindingDescriptions;
	}
	inline const MVector<VkVertexInputAttributeDescription>& GetAttributeDescriptions()
	{
		return _attributeDescriptions;
	}
	inline const uint32_t GetMeshCount() { return _meshDatas.size(); }
	inline const VKMeshData& GetMeshData(uint32_t index) { return _meshDatas[index]; }
private:
	VKMesh(const char* path);
	VKMesh(MDefaultMesh defaultMesh);
	void SetDescriptionModel();
	void SetDescriptionDefault();
	void FillData(const char* path);
	void FillData(MDefaultMesh defaultMesh);
private:
	glm::mat4 _model;
	MVector<VKMeshData> _meshDatas;
	MVector<VkVertexInputBindingDescription> _bindingDescriptions;
	MVector<VkVertexInputAttributeDescription> _attributeDescriptions;
};

class VKMeshManager : public GC
{
public:
	VKMesh* CreateMeshDefault(MDefaultMesh defaultMesh);
	VKMesh* CreateMeshModel(const char* path);
private:
	MUMap<std::string, VKMesh*> _cache;
};

MORISA_NAMESPACE_END

#endif