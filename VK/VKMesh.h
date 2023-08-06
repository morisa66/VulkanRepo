#ifndef __VK_MESH_H__
#define __VK_MESH_H__

#include "VKInclude.h"
#include "Utils/MatrixUtils.h"
#include "Utils/ModelUtils.h"
#include "Core/GC.h"

#include<vector>

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
	inline const std::vector<VkVertexInputBindingDescription>& GetBindingDescriptions()
	{
		return _bindingDescriptions;
	}
	inline const std::vector<VkVertexInputAttributeDescription>& GetAttributeDescriptions()
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
	std::vector<VKMeshData> _meshDatas;
	std::vector<VkVertexInputBindingDescription> _bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> _attributeDescriptions;
};

class VKMeshManager : public GC
{
public:
	VKMesh* CreateMeshDefault(MDefaultMesh defaultMesh);
	VKMesh* CreateMeshModel(const char* path);
private:

};

MORISA_NAMESPACE_END

#endif