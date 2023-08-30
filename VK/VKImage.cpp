#include "VKImage.h"
#include "VKContext.h"
#include "VKBuffer.h"
#include "VKCommandBuffer.h"
#include "VKSynchronization.h"

#include <array>

MORISA_NAMESPACE_BEGIN
VKImage* VKImage::White = nullptr;

VKImage::VKImage(const VKImageInfo* info):
	_info(info)
	, _image(VK_NULL_HANDLE)
	, _view(VK_NULL_HANDLE)
	, _sampler(VK_NULL_HANDLE)
{
	VkDevice device = Device();

	VK_STRUCT_CREATE(VkImageCreateInfo, imageCreateInfo, VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO);
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = _info.format;
	imageCreateInfo.extent = VkExtent3D{ _info.width, _info.height, 1 };
	imageCreateInfo.mipLevels = info->mipLevels;
	imageCreateInfo.arrayLayers = 1;
	// AA
	imageCreateInfo.samples = info->samples;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = info->usage;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	// Multi queue used
	imageCreateInfo.queueFamilyIndexCount = 0;
	imageCreateInfo.pQueueFamilyIndices = nullptr;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VK_RESULT_CHECK(vkCreateImage, device, &imageCreateInfo, nullptr, &_image);

	VkMemoryRequirements memoryRequirements{};
	vkGetImageMemoryRequirements(device, _image, &memoryRequirements);

	VkMemoryAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = Context()->BufferManager()->GetMemoryIndex(
		memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VK_RESULT_CHECK(vkAllocateMemory, device, &allocateInfo, nullptr, &_memory);

	VK_RESULT_CHECK(vkBindImageMemory, device, _image, _memory, 0);
}

VKImage::~VKImage()
{
	VkDevice device = Device();
	VK_VALID_DESTROY(_view, vkDestroyImageView, device, _view, nullptr);
	VK_VALID_DESTROY(_image, vkDestroyImage, device, _image, nullptr);
	VK_VALID_DESTROY(_sampler, vkDestroySampler, device, _sampler, nullptr);
	VK_VALID_DESTROY(_memory, vkFreeMemory, device, _memory, nullptr);
}


VKImageManager::VKImageManager():
	_deviceProperties(Context()->DeviceProperties())
{
	const VkPhysicalDeviceLimits& limits = _deviceProperties.limits;
	_maxSamplerAnisotropy = limits.maxSamplerAnisotropy;

	VkSampleCountFlags sampleCounts = limits.framebufferColorSampleCounts & limits.framebufferDepthSampleCounts;
	std::array<VkSampleCountFlagBits, 6> msaaSamplesArray =
	{
		VK_SAMPLE_COUNT_64_BIT,
		VK_SAMPLE_COUNT_32_BIT,
		VK_SAMPLE_COUNT_16_BIT,
		VK_SAMPLE_COUNT_8_BIT,
		VK_SAMPLE_COUNT_4_BIT,
		VK_SAMPLE_COUNT_2_BIT,
	};
	_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
	for (VkSampleCountFlagBits bit : msaaSamplesArray)
	{
		if (sampleCounts & bit)
		{
			_msaaSamples = bit;
			break;
		}
	}

	VKImage::White = CreateImageFromPath("/Pictures/Default/White.png");
}

VkImageView VKImageManager::CreateImageView(VKImage* image)
{
	VkImageView view = VK_NULL_HANDLE;
	VK_STRUCT_CREATE(VkImageViewCreateInfo, createInfo, VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
	createInfo.image = image->Access();
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = image->Info().format;
	createInfo.components =
	{
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
	};
	VkImageAspectFlags aspect = image->Info().aspect;
	createInfo.subresourceRange = { aspect , 0, image->Info().mipLevels, 0, 1};
	VK_RESULT_CHECK(vkCreateImageView, Device(), &createInfo, nullptr, &view);
	
	return view;
}

VkSampler VKImageManager::CreateSampler(VKImage* image)
{
	VkSampler sampler = VK_NULL_HANDLE;
	VK_STRUCT_CREATE(VkSamplerCreateInfo, createInfo, VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO);
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.addressModeU = image->Info().addressMode;
	createInfo.addressModeV = image->Info().addressMode;
	createInfo.addressModeW = image->Info().addressMode;
	createInfo.mipLodBias = 0;
	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = min(1.0f, _maxSamplerAnisotropy);
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	if (image->Info().mipLevels > 1)
	{
		createInfo.minLod = 0.0f;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		createInfo.maxLod = (float)image->Info().mipLevels;
	}
	else
	{
		createInfo.minLod = 0.0f;
		createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		createInfo.maxLod = 0.0f;
	}
	createInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	VK_RESULT_CHECK(vkCreateSampler, Device(), &createInfo, nullptr, &sampler);

	return sampler;
}


VKImage* VKImageManager::CreateImage(const VKImageInfo* info)
{
	VKImage* image = MORISA_NEW(VKImage, info);

	VKContext* context = Context();

	if (info->image != nullptr)
	{
		// TODO check image is valid
		VKBuffer* imageDataBuffer = context->BufferManager()->CreateGCBuffer(
			info->image->Size(),
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT, false, info->image->Access());

		// memory copy to buffer, free the read memory, maybe reuse
		//info->image->FreeImageReadMemory();
		VKCommandBuffer* commandBuffer = context->CommandBuffer();
		commandBuffer->Begin();
		commandBuffer->TransitionLayout(image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		commandBuffer->CopyBufferToImage(imageDataBuffer, image);
		if (info->mipLevels > 1 && SupportLinearBlit(info->format))
		{
			CreateMips(commandBuffer, image);
		}
		else
		{
			commandBuffer->TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}
		commandBuffer->End();
		// TODO Synchronization to make sure the image is accessible
		commandBuffer->Submit(context->Synchronization()->GetFreeFence());
	}


	image->_view = CreateImageView(image);
	if (info->usage & VK_IMAGE_USAGE_SAMPLED_BIT)
	{
		image->_sampler = CreateSampler(image);
	}

	PushPersistence(image);

	return image;
}

VKImage* VKImageManager::CreateImageFormImage(MImage* mImage)
{
	if (mImage == nullptr)
	{
		return nullptr;
	}

	VKImageInfo imageInfo;
	imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.width = mImage->Width();
	imageInfo.height = mImage->Height();
	imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.aspect = VK_IMAGE_ASPECT_COLOR_BIT;
	imageInfo.image = mImage;
	imageInfo.mipLevels = mImage->MipLevels();
	imageInfo.addressMode = VK_SAMPLER_ADDRESS_MODE_REPEAT;

	VKImage* image = CreateImage(&imageInfo);
	return image;
}

VKImage* VKImageManager::CreateImageFromPath(const char* path)
{
	return CreateImageFormImage(MORISA_NEW(MImage, path));
}

void VKImageManager::CreateMips(VKCommandBuffer* commandBuffer, VKImage* image)
{
	const VKImageInfo& info = image->Info();
	for (uint32_t i = 0; i < info.mipLevels - 1; i++)
	{
		commandBuffer->TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, i, 1);
		// Blit TransitionLayout mip map to VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		commandBuffer->BlitMipImage(image, i);
		commandBuffer->TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, i, 1);
	}
	commandBuffer->TransitionLayout(image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, info.mipLevels - 1, 1);
}

bool VKImageManager::SupportLinearBlit(VkFormat format)
{
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(Context()->PhysicalDevice(), format, &formatProperties);
	if (!(formatProperties.linearTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
	{
		MORISA_LOG("VkFormat %d not support linearTilingFeatures\n", format);
		return false;
	}
	return true;
}

MORISA_NAMESPACE_END

