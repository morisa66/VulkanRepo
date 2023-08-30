#ifndef __VK_MESH_H__
#define __VK_MESH_H__

#include "VKInclude.h"
#include "Utils/MatrixUtils.h"
#include "Utils/ModelUtils.h"
#include "Core/GC.h"

#include "Core/MMap.h"
#include "Core/MVector.h"
#include "Core/MString.h"

MORISA_NAMESPACE_BEGIN

class VKBuffer;
class VKImage;

enum MDefaultMesh
{
	kMDefaultMeshPlane,
	kMDefaultMeshQuad,
	kMDefaultMeshCube,
	kMDefaultMeshCount,
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
	MVector<uint32_t> imageIndices;
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
	const MMap<MString, MVector<VKImage*>>& GetImages() { return _images; }
private:
	VKMesh(MMesh* mMesh);
	VKMesh(const char* path);
	void SetDescription();
	void FillMeshData(VKMeshData& meshData, MMesh* mMesh);
	void FillData(const char* path);
private:
	MVector<VKMeshData> _meshDatas;
 	MMap<MString, MVector<VKImage*>> _images;
	MVector<VkVertexInputBindingDescription> _bindingDescriptions;
	MVector<VkVertexInputAttributeDescription> _attributeDescriptions;
};

class VKMeshManager : public GC
{
public:
	VKMeshManager();
	VKMesh* CreateMeshDefault(MDefaultMesh defaultMesh);
	VKMesh* CreateMeshModel(const char* path);
private:
	VKMesh* GenerateMeshDefault(MMesh* mMesh);
private:
	MUMap<MDefaultMesh, VKMesh*> _cacheDefaultMeshs;
	MUMap<MString, VKMesh*> _cacheModelMeshs;
};

MORISA_NAMESPACE_END

#endif