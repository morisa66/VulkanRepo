#ifndef __VK_SWAPCHAIN_H__
#define __VK_SWAPCHAIN_H__

#include "Core/MVector.h"

#include "VKInclude.h"

MORISA_NAMESPACE_BEGIN

class VKContext;

struct VKSwapChainImageInfo
{
	MVector<VkImage> images;
	MVector<VkImageView> views;
	VkFormat format;
	uint32_t currentIndex;
};

class VKSwapChain
{
public:
	VKSwapChain();
	~VKSwapChain();
	inline VkSwapchainKHR Swapchain() { return _swapChain; }
	inline const VKSwapChainImageInfo& GetSwapChainImageInfo() { return _swapChainImageInfo; }
	uint32_t GetSwapChainImageIndex(VkSemaphore semaphore, VkFence fence);
	void Present(VkSemaphore semaphore);
	inline const VkExtent2D& GetExtent() { return _extent; }
private:
	void ChooseSurfaceFormat();
	void ChoosePresentMode();
	void SetSwapChainInfo();
	void CreateSwapChain();
	void GetSwapchainImage();
private:
	VkPhysicalDevice _physicalDevice;
	VkQueue _presentQueue;

	VkSwapchainKHR _swapChain;
	VkSwapchainKHR _oldSwapChain;
	VkSurfaceKHR _surface;
	VkColorSpaceKHR _colorSpace;
	VkPresentModeKHR _presentMode;
	VkExtent2D _extent;
	uint32_t _imageCount;
	VkSurfaceTransformFlagBitsKHR _preTransform;
	
	VKSwapChainImageInfo _swapChainImageInfo;
};

MORISA_NAMESPACE_END

#endif
