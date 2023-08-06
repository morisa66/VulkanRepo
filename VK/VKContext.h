#pragma once
#ifndef __VK_CONTEXT_H__
#define __VK_CONTEXT_H__

#include "VKInclude.h"
#include "VKWindow.h"
#include "VKSwapChain.h"


MORISA_NAMESPACE_BEGIN

class VKBufferManager;
class VKImageManager;
class VKCommandBuffer;
class VKSynchronization;
class VKDescriptorManager;
class VKUniform;
class VKUniformManager;
class VKMeshManager;
class VKPipelineCache;
class VKRenderer;
struct EventProcessorData;
class MMaterial;

class VKContext
{
public:
	VKContext();
	~VKContext();
	void Run() { _window->RenderLoop(); }
	inline VkInstance Instance() { return _instance; }
	inline VkPhysicalDevice PhysicalDevice() { return _physicalDevice; }
	inline VkDevice Device() { return _device; }
	inline VkSurfaceKHR Surface() { return _window->Surface(); }
	inline const VkExtent2D& WindowSize() { return _window->Size(); }
	inline void SetWindowSize(const VkExtent2D& size) { _window->SetSize(size); }
	inline VkSwapchainKHR SwapChain() { return _swapChain->Swapchain(); }
	inline const VKSwapChainImageInfo& GetSwapChainImageInfo() { return _swapChain->GetSwapChainImageInfo(); }
	inline uint32_t GetSwapChainImageIndex(VkSemaphore semaphore, VkFence fence = VK_NULL_HANDLE) { return _swapChain->GetSwapChainImageIndex(semaphore, fence); }
	inline const uint32_t GraphicsQueueIndex() { return _graphicsQueueIndex; }
	inline const VkQueue GraphicsQueue() { return _queue; }
	inline void Present(VkSemaphore semaphore) { _swapChain->Present(semaphore); }
	
	inline VKBufferManager* BufferManager() { return _bufferManager; }
	inline VKImageManager* ImageManager() { return _imageManager; }
	inline VKCommandBuffer* CommandBuffer() { return _commandBuffer; }
	inline VKDescriptorManager* DescriptorManager() { return _descriptorManager; }
	inline VKSynchronization* Synchronization() { return _synchronization; }
	inline VKUniformManager* UniformManager() { return _uniformManager; }
	inline VKMeshManager* MeshManager() { return _meshManager; }
	inline VKUniform* GlobalUniform() { return _globalUniform; }
	inline MMaterial* GlobalMaterial() { return _globalMaterial; }
	inline VKPipelineCache* PipelineCache() { return _pipelineCache; }

	inline const VkPhysicalDeviceProperties& DeviceProperties() { return _deviceProperties; }

	inline void SetEventProcessorData(EventProcessorData* eventProcessorData) { _window->SetEventProcessorData(eventProcessorData);}

	void WaitIdle();
	void WindowResize();
private:
	void CreateInstance();
	void PickUpGpu();
	void SetQueueFamilies();
	void CreateDevice();
	void GetQueue();
	void InitGlobalUniform();
private:
	VkInstance _instance;
	VkDevice _device;
	VkPhysicalDevice _physicalDevice;
	VkPhysicalDeviceProperties _deviceProperties;
	VkPhysicalDeviceFeatures _deviceFeatures;
	uint32_t _graphicsQueueIndex;
	uint32_t _computeQueueIndex;
	uint32_t _transferQueueIndex;
	uint32_t _presentQueueIndex;
	// TODO 
	// Multi queue
	VkQueue _queue;

	VKWindow* _window;
	VKSwapChain* _swapChain;
	VKBufferManager* _bufferManager;
	VKImageManager* _imageManager;
	VKCommandBuffer* _commandBuffer;
	VKSynchronization* _synchronization;
	VKDescriptorManager* _descriptorManager;
	VKUniformManager* _uniformManager;
	VKMeshManager* _meshManager;
	VKPipelineCache* _pipelineCache;

	MMaterial* _globalMaterial;
	VKUniform* _globalUniform;

	VKRenderer* _renderer;
};

VKContext* Context();
VkDevice Device();

MORISA_NAMESPACE_END

#endif