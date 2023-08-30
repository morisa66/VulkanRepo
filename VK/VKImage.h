#ifndef __VK_IMAGE_H__
#define __VK_IMAGE_H__

#include "VKInclude.h"
#include "VKTools.h"
#include "Core/GC.h"
#include "Utils/ImageUtils.h"

#include <queue>

class VKBuffer;

MORISA_NAMESPACE_BEGIN

class VKImageManager;
class VKCommandBuffer;

struct VKImageInfo
{
	VkFormat format;
	uint32_t width;
	uint32_t height;
	VkImageUsageFlags usage;
	VkImageAspectFlags aspect;
	VkSampleCountFlagBits samples;
	VkSamplerAddressMode addressMode;
	uint32_t mipLevels;
	MImage* image;
	uint32_t depthBit : 1;
	VKImageInfo() :
		format(VK_FORMAT_UNDEFINED)
		, width(0)
		, height(0)
		, usage(0)
		, aspect(0)
		, samples(VK_SAMPLE_COUNT_1_BIT)
		, addressMode(VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE)
		, mipLevels(1)
		, image(nullptr)
		, depthBit(0)
	{
	}

	VKImageInfo(const VKImageInfo* info)
	{
		memcpy(this, info, sizeof(VKImageInfo));
	}
};

class VKImage : public GCNode
{
	friend VKImageManager;
public:
	~VKImage();
	const VKImageInfo& Info() { return _info; }
	VkImage Access() { return _image; }
	VkImageView AccessView() { return _view; }
	VkSampler AccessSampler() { return _sampler; }
	VkImageLayout AccessDescriptorLayout() 
	{
		return _info.depthBit ?
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	}
	//SwapChainImage is only a placeholder.
	static VKImage* SwapChainImage() { return nullptr; }
	static VKImage* White;
private:
	VKImage(const VKImageInfo* info);
private:
	VKImageInfo _info;
	VkImage _image;
	VkDeviceMemory _memory;
	VkImageView _view;
	VkSampler _sampler;
};

class VKImageManager : public GC
{
public:
	VKImageManager();
	VKImage* CreateImage(const VKImageInfo* info);
	VKImage* CreateImageFormImage(MImage* mImage);
	VKImage* CreateImageFromPath(const char* path);
	VkSampleCountFlagBits MsaaSamples() { return _msaaSamples; }
	bool SupportLinearBlit(VkFormat format);
private:
	VkImageView CreateImageView(VKImage* image);
	VkSampler CreateSampler(VKImage* image);
	void CreateMips(VKCommandBuffer* commandBuffer, VKImage* image);
private:
	const VkPhysicalDeviceProperties& _deviceProperties;
	float _maxSamplerAnisotropy;
	VkSampleCountFlagBits _msaaSamples;
};

MORISA_NAMESPACE_END

#endif
