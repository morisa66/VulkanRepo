#include <algorithm> // std::clamp C++17

#include "VKSwapChain.h"
#include "VKContext.h"

MORISA_NAMESPACE_BEGIN

VKSwapChain::VKSwapChain():
	_physicalDevice(Context()->PhysicalDevice())
	, _surface(Context()->Surface())
	, _presentQueue(Context()->GraphicsQueue())
	,_extent(Context()->WindowSize())
	, _imageCount(0)
	, _swapChain(VK_NULL_HANDLE)
	, _oldSwapChain(VK_NULL_HANDLE)
	, _preTransform(VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
{
	ChooseSurfaceFormat();
	ChoosePresentMode();
	SetSwapChainInfo();

	CreateSwapChain();

	GetSwapchainImage();
}

VKSwapChain::~VKSwapChain()
{
	VkDevice device = Device();
	for (VkImageView view : _swapChainImageInfo.views)
	{
		VK_VALID_DESTROY(view, vkDestroyImageView, device, view, nullptr);
	}
	VK_VALID_DESTROY(_swapChain, vkDestroySwapchainKHR, device, _swapChain, nullptr);
}

uint32_t VKSwapChain::GetSwapChainImageIndex(VkSemaphore semaphore, VkFence fence)
{
	VK_RESULT_CHECK(vkAcquireNextImageKHR, Device(), _swapChain, UINT64_MAX, semaphore, fence, &_swapChainImageInfo.currentIndex);
	return _swapChainImageInfo.currentIndex;
}


void VKSwapChain::Present(VkSemaphore semaphore)
{
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.pNext = nullptr;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &semaphore;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapChain;
	presentInfo.pImageIndices = &_swapChainImageInfo.currentIndex;
	presentInfo.pResults = nullptr;

	VK_RESULT_CHECK(vkQueuePresentKHR, _presentQueue, &presentInfo);
}

void VKSwapChain::ChooseSurfaceFormat()
{
	uint32_t count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &count, nullptr);
	std::vector<VkSurfaceFormatKHR> formats(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _surface, &count, formats.data());
	for (const VkSurfaceFormatKHR& surfaceFormat : formats)
	{
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			_swapChainImageInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
			_colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
			return;
		}
	}
	_swapChainImageInfo.format = formats.front().format;
	_colorSpace = formats.front().colorSpace;
}

void VKSwapChain::ChoosePresentMode()
{
	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, nullptr);
	std::vector<VkPresentModeKHR> presentModes(presentModeCount);
	vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _surface, &presentModeCount, presentModes.data());
	
	for (VkPresentModeKHR mode : presentModes)
	{
		// PC
		if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
		{
			_presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
			return;
		}
	}
	_presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void VKSwapChain::SetSwapChainInfo()
{
	VkSurfaceCapabilitiesKHR capabilities{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _surface, &capabilities);

	_imageCount = min(capabilities.minImageCount + 1, capabilities.maxImageCount);

	_preTransform = capabilities.currentTransform;
	if (capabilities.currentExtent.width != UINT32_MAX) 
	{
		_extent = capabilities.currentExtent;
	}
	else
	{
		_extent.width = std::clamp(_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		_extent.height = std::clamp(_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
	}
}

void VKSwapChain::CreateSwapChain()
{
	VkSwapchainCreateInfoKHR swapchainCreateInfo{};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.pNext = nullptr;
	swapchainCreateInfo.flags = 0;
	swapchainCreateInfo.surface = _surface;

	swapchainCreateInfo.minImageCount = _imageCount;
	swapchainCreateInfo.imageFormat = _swapChainImageInfo.format;
	swapchainCreateInfo.imageColorSpace = _colorSpace;
	swapchainCreateInfo.imageExtent = _extent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// _graphicsQueueIndex == _presentQueueIndex
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.queueFamilyIndexCount = 0;
	swapchainCreateInfo.pQueueFamilyIndices = nullptr;

	swapchainCreateInfo.preTransform = _preTransform;
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainCreateInfo.presentMode = _presentMode;
	swapchainCreateInfo.clipped = VK_TRUE;

	swapchainCreateInfo.oldSwapchain = _oldSwapChain;

	VK_RESULT_CHECK(vkCreateSwapchainKHR, Device(), &swapchainCreateInfo, nullptr, &_swapChain);
}

void VKSwapChain::GetSwapchainImage()
{
	uint32_t count = 0;
	VkDevice device = Device();
	vkGetSwapchainImagesKHR(device, _swapChain, &count, nullptr);
	_swapChainImageInfo.images.resize(count);
	_swapChainImageInfo.views.resize(count);
	vkGetSwapchainImagesKHR(device, _swapChain, &count, _swapChainImageInfo.images.data());

	VK_STRUCT_CREATE(VkImageViewCreateInfo, imageViewCreateInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	for (size_t i = 0; i < count; ++i)
	{
		imageViewCreateInfo.image = _swapChainImageInfo.images[i];
		imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		imageViewCreateInfo.format = _swapChainImageInfo.format;
		imageViewCreateInfo.components = 
		{ 
			VK_COMPONENT_SWIZZLE_IDENTITY, 
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY, 
			VK_COMPONENT_SWIZZLE_IDENTITY,
		};
		imageViewCreateInfo.subresourceRange =
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		};

		VK_RESULT_CHECK(vkCreateImageView, Device(), &imageViewCreateInfo, nullptr, &_swapChainImageInfo.views[i]);
	}
	_swapChainImageInfo.currentIndex = 0;
}


MORISA_NAMESPACE_END

