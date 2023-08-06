#include "VKBuffer.h"
#include "VKContext.h"
#include "VKCommandBuffer.h"
#include "VKSynchronization.h"

MORISA_NAMESPACE_BEGIN

VKBuffer::VKBuffer(VKBufferManager* manager, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags flags):
   _size(size)
   , _usage(usage)
   , _data(nullptr)
   , _buffer(VK_NULL_HANDLE)
   , _memory(VK_NULL_HANDLE)

{
    VkDevice device = Device();
    VK_STRUCT_CREATE(VkBufferCreateInfo, createInfo, VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
    createInfo.size = size;
    createInfo.usage = usage;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    // Single queue use
    createInfo.queueFamilyIndexCount = manager->_indices.size();
    createInfo.pQueueFamilyIndices = manager->_indices.data();

    VK_RESULT_CHECK(vkCreateBuffer, device, &createInfo, nullptr, &_buffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(device, _buffer, &memoryRequirements);

    VkMemoryAllocateInfo allocateInfo{};
    allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocateInfo.pNext = nullptr;
    allocateInfo.allocationSize = memoryRequirements.size;
    allocateInfo.memoryTypeIndex = manager->GetMemoryIndex(memoryRequirements.memoryTypeBits, flags);

    // TODO Allocate Big memory first, each get with offset.
    VK_RESULT_CHECK(vkAllocateMemory, device, &allocateInfo, nullptr, &_memory);

    VK_RESULT_CHECK(vkBindBufferMemory, device, _buffer, _memory, 0);
}

VKBuffer::~VKBuffer()
{
    VkDevice device = Device();
    VK_VALID_DESTROY(_buffer, vkDestroyBuffer, device, _buffer, nullptr);
    VK_VALID_DESTROY(_memory, vkFreeMemory, device, _memory, nullptr);
}

void VKBuffer::Map(const void* data, VkDeviceSize size)
{
    if (_size < size)
    {
        MORISA_LOG("VKBuffer::Map size too Large(%lld < %lld)", _size, size);
        return;
    }
    VkDevice device = Device();
    //TODO remove memcpy
    VK_RESULT_CHECK(vkMapMemory, device, _memory, 0, size, 0, &_data);
    memcpy(_data, data, size);
    vkUnmapMemory(device, _memory);
}


VKBufferManager::VKBufferManager()
{
    _indices.emplace_back(Context()->GraphicsQueueIndex());
    vkGetPhysicalDeviceMemoryProperties(Context()->PhysicalDevice(), &_memoryProperties);
}

VKBuffer* VKBufferManager::CreatePersistenceBuffer(
    VkDeviceSize size, 
    VkBufferUsageFlags usage,
    const bool useStage,
    const void* data)
{
    return CreateBuffer(size, usage, useStage, data, true);
}

VKBuffer* VKBufferManager::CreateGCBuffer(VkDeviceSize size, VkBufferUsageFlags usage, const bool useStage, const void* data)
{
    return CreateBuffer(size, usage, useStage, data, false);
}

uint32_t VKBufferManager::GetMemoryIndex(uint32_t types, VkMemoryPropertyFlags flags)
{
    for (uint32_t i = 0; i < _memoryProperties.memoryTypeCount; i++) 
    {
        if ((types & (1 << i)) 
            && (_memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
        {
            return i;
        }
    }

    MORISA_LOG("VKBufferManager::GetMemoryIndex ERROR\n");
    return UINT32_MAX;
}

VKBuffer* VKBufferManager::CreateBuffer(
    VkDeviceSize size,
    VkBufferUsageFlags usage,
    const bool useStage,
    const void* data,
    const bool persistence)
{
    VKBuffer* buffer = nullptr;
    VKBuffer* stageBuffer = nullptr;
    VkFence fence = VK_NULL_HANDLE;
    if (useStage)
    {
        fence = Context()->Synchronization()->GetFreeFence();
        buffer = MORISA_NEW(VKBuffer, this, size, usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        stageBuffer = MORISA_NEW(VKBuffer, this, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        PushGC((uint64_t)fence, stageBuffer);
    }
    else
    {
        buffer = MORISA_NEW(VKBuffer, this, size, usage,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    }

    if (persistence)
    {
        PushPersistence(buffer);
    }
    else
    {
        PushGC((uint64_t)fence, buffer);
    }

    if (useStage)
    {
        stageBuffer->Map(data, size);
    }
    else
    {
        buffer->Map(data, size);
    }

    if (useStage)
    {
        VKCommandBuffer* commandBuffer = Context()->CommandBuffer();
        commandBuffer->Begin();
        commandBuffer->CopyBuffer(stageBuffer, buffer);
        commandBuffer->End();
        // TODO need to know what time the resouces can be used.
        commandBuffer->Submit(fence);
    }

    return buffer;
}

MORISA_NAMESPACE_END


