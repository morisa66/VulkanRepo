#include "VKSynchronization.h"
#include "VKContext.h"
#include "VKUniform.h"
#include "VKDescriptor.h"
#include "VKBuffer.h"
#include "VKImage.h"

MORISA_NAMESPACE_BEGIN

VKSynchronization::VKSynchronization()
{
	CreateParallelSyncResouces();
}

VKSynchronization::~VKSynchronization()
{
	VkDevice device = Device();
	DestroyParallelSyncResouces();

	DESTROY_POOL(VkSemaphore, _semaphoreFreePool, vkDestroySemaphore, 
		device, VkSemaphoreHANDLE, nullptr);
	DESTROY_POOL(VkSemaphore, _semaphoreUsedPool, vkDestroySemaphore,
		device, VkSemaphoreHANDLE, nullptr);
	DESTROY_POOL(VkFence, _fenceFreePool, vkDestroyFence,
		device, VkFenceHANDLE, nullptr);
	DESTROY_POOL(VkFence, _fenceUsedPool, vkDestroyFence,
		device, VkFenceHANDLE, nullptr);
}

VkSemaphore VKSynchronization::GetFreeSemaphore()
{
	VkSemaphore semaphore = VK_NULL_HANDLE;
	if (!_semaphoreFreePool.empty())
	{
		semaphore = _semaphoreFreePool.front();
		_semaphoreFreePool.pop();
	}
	else
	{
		VK_STRUCT_CREATE(VkSemaphoreCreateInfo, createInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
		VK_RESULT_CHECK(vkCreateSemaphore, Device(), &createInfo, nullptr, &semaphore);
	}
	_semaphoreUsedPool.push(semaphore);
	return semaphore;
}

VkFence VKSynchronization::GetFreeFence()
{
	VkFence fence = VK_NULL_HANDLE;
	if (!_fenceFreePool.empty())
	{
		fence = _fenceFreePool.front();
		_fenceFreePool.pop();
	}
	else
	{
		VK_STRUCT_CREATE(VkFenceCreateInfo, createInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
		VK_RESULT_CHECK(vkCreateFence, Device(), &createInfo, nullptr, &fence);
	}
	_fenceUsedPool.push(fence);
	return fence;
}

void VKSynchronization::SyncLast()
{
	VkDevice device = Device();
	VkFence fence = _infos[_version].fence;
	vkWaitForFences(device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkResetFences(device, 1, &fence);

	VKBufferManager* bufferManager = Context()->BufferManager();
	VKImageManager* imageManager = Context()->ImageManager();

	bufferManager->DelayDestroy();
	imageManager->DelayDestroy();

	while (!_fenceUsedPool.empty()
		&& vkGetFenceStatus(device, _fenceUsedPool.front()) == VK_SUCCESS)
	{
		VkFence fence = _fenceUsedPool.front();
		_fenceUsedPool.pop();
		bufferManager->Destroy((uint64_t)fence);
		imageManager->Destroy((uint64_t)fence);
		vkResetFences(device, 1, &fence);
		_fenceFreePool.push(fence);
	}
}

void VKSynchronization::Flush()
{
	Version::Flush((_version + 1) % MAX_PARALLEL_FRAME_VERSION);

	Context()->UniformManager()->Flush(_version);
}

void VKSynchronization::CreateParallelSyncResouces()
{
	VkDevice device = Device();
	VK_STRUCT_CREATE(VkFenceCreateInfo, fenceCreateInfo, VK_STRUCTURE_TYPE_FENCE_CREATE_INFO);
	VK_STRUCT_CREATE(VkSemaphoreCreateInfo, semaphoreCreateInfo, VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO);
	// the first frame not need to wait anything, so signal is true
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (int i = 0; i < MAX_PARALLEL_FRAME_VERSION; ++i)
	{
		VK_RESULT_CHECK(vkCreateFence, device, &fenceCreateInfo, nullptr, &_infos[i].fence);
		VK_RESULT_CHECK(vkCreateSemaphore, device, &semaphoreCreateInfo, nullptr, &_infos[i].imageAvailableSemaphore);
		VK_RESULT_CHECK(vkCreateSemaphore, device, &semaphoreCreateInfo, nullptr, &_infos[i].renderFinishedSemaphore);
	}
}

void VKSynchronization::DestroyParallelSyncResouces()
{
	// Reuse
	for (int i = 0; i < MAX_PARALLEL_FRAME_VERSION; ++i)
	{
		vkResetFences(Device(), 1, &_infos[i].fence);
		_fenceFreePool.push(_infos[i].fence);
		_infos[i].fence = VK_NULL_HANDLE;
		_semaphoreFreePool.push(_infos[i].imageAvailableSemaphore);
		_infos[i].imageAvailableSemaphore = VK_NULL_HANDLE;
		_semaphoreFreePool.push(_infos[i].renderFinishedSemaphore);
		_infos[i].renderFinishedSemaphore = VK_NULL_HANDLE;
	}
}


MORISA_NAMESPACE_END


