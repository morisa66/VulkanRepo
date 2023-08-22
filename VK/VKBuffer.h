#include "VKInclude.h"
#include "Core/GC.h"
#include "VKTools.h"

#include "Core/MVector.h"
#include <queue>
#include <map>

MORISA_NAMESPACE_BEGIN

class VKCommandBuffer;
class VKSynchronization;
class VKBufferManager;

class VKBuffer : public GCNode
{
	friend class VKBufferManager;
public:
	~VKBuffer();
	inline const VkBuffer Access() { return _buffer; }
	inline const VkBuffer* AccessPtr() { return &_buffer; }
	inline void* AccessData() { return _data; }
	inline const VkDeviceSize Size() { return _size; }
	void Map(const void* data, VkDeviceSize size);
private:
	VKBuffer(VKBufferManager* manager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags);
private:
	VkDeviceSize _size;
	VkBufferUsageFlags _usage;
	void* _data;
	VkBuffer _buffer;
	VkDeviceMemory _memory;
};

class VKBufferManager : public GC
{
	friend class VKBuffer;
public:
	VKBufferManager();
	
	// Vertex or index buffer useStage = true, uniform buffer useStage =false...
	VKBuffer* CreatePersistenceBuffer(
		VkDeviceSize size, 
		VkBufferUsageFlags usage, 
		const bool useStage,
		const void* data);

	VKBuffer* CreateGCBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		const bool useStage,
		const void* data);

	uint32_t GetMemoryIndex(uint32_t types, VkMemoryPropertyFlags flags);

private:
	VKBuffer* CreateBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		const bool useStage,
		const void* data,
		const bool persistence);
private:
	VkPhysicalDeviceMemoryProperties _memoryProperties;
	MVector<uint32_t> _indices;
};

MORISA_NAMESPACE_END