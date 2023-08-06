#ifndef __VK_SYNCHRONIZATION_H__
#define __VK_SYNCHRONIZATION_H__

#include "VKInclude.h"
#include "Core/Version.h"

#include <queue>

MORISA_NAMESPACE_BEGIN

class VKBufferManager;
class VKImageManager;


struct VKSynchronizationInfo
{
	VkFence fence;
	VkSemaphore imageAvailableSemaphore;
	VkSemaphore renderFinishedSemaphore;
};

class VKSynchronization : public Version
{
public:
	VKSynchronization();
	~VKSynchronization();
	const VKSynchronizationInfo* GetCurrentInfo() { return &_infos[_version]; }
	VkSemaphore GetFreeSemaphore();
	VkFence GetFreeFence();
	void SyncLast();
	void Flush();
	uint32_t GetSyncIndex() { return _version; }
private:
	void CreateParallelSyncResouces();
	void DestroyParallelSyncResouces();
private:
	VKSynchronizationInfo _infos[MAX_PARALLEL_FRAME_VERSION];
	std::queue<VkSemaphore> _semaphoreFreePool;
	std::queue<VkFence> _fenceFreePool;
	std::queue<VkSemaphore> _semaphoreUsedPool;
	std::queue<VkFence> _fenceUsedPool;
};

MORISA_NAMESPACE_END

#endif
