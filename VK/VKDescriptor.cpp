#include "VKDescriptor.h"
#include "VKContext.h"
#include "VKUniform.h"
#include "VKBuffer.h"
#include "VKImage.h"

#include "Runtime/Graphics/Material.h"

MORISA_NAMESPACE_BEGIN

#define MAX_TYPE_COUNT 256
#define MAX_SET_COUNT 256

VKDescriptorSet::VKDescriptorSet()
{
}

VKDescriptorSet::~VKDescriptorSet()
{
	VK_VALID_DESTROY(_layouts[0], vkDestroyDescriptorSetLayout, Device(), _layouts[0], nullptr);
}


VKDescriptorManager::VKDescriptorManager():
	_pool(VK_NULL_HANDLE)
{
	CreatePool();
}


VKDescriptorManager::~VKDescriptorManager()
{
	VK_VALID_DESTROY(_pool, vkDestroyDescriptorPool, Device(), _pool, nullptr);
}


VKDescriptorSet* VKDescriptorManager::CreateDescriptorSet(VKUniform* uniform)
{
	VKDescriptorSet* set = MORISA_NEW(VKDescriptorSet);
	MMaterial* material = uniform->Material();
	CreateLayout(material, set->_layouts);

	AllocateSets(set->_layouts, set->_sets);

	UpdateSets(uniform, set->_sets);

	PushPersistence(set);

	return set;
}

void VKDescriptorManager::Update(VKUniform* uniform)
{
	uniform->Material()->FlushData();
	UpdateSets(uniform, uniform->DescriptorSet()->_sets);
}

void VKDescriptorManager::CreatePool()
{
	MVector<VkDescriptorPoolSize> poolSizes(2);
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = MAX_TYPE_COUNT;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = MAX_TYPE_COUNT;

	VK_STRUCT_CREATE(VkDescriptorPoolCreateInfo, createInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO);
	createInfo.maxSets = MAX_SET_COUNT;
	createInfo.poolSizeCount = poolSizes.size();
	createInfo.pPoolSizes = poolSizes.data();

	VK_RESULT_CHECK(vkCreateDescriptorPool, Device(), &createInfo, nullptr, &_pool);
}

void VKDescriptorManager::CreateLayout(MMaterial* material, 
	VersionArray<VkDescriptorSetLayout>& layouts)
{
	static const VkShaderStageFlags stageFlags[kMShaderStageCount] 
	= { VK_SHADER_STAGE_VERTEX_BIT ,VK_SHADER_STAGE_FRAGMENT_BIT };

	VkDescriptorSetLayout layout = VK_NULL_HANDLE;
	MVector<VkDescriptorSetLayoutBinding> layoutBindings;

	uint32_t bindingIndex = 0;
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		VkDescriptorSetLayoutBinding uniformBinding;
		uniformBinding.binding = bindingIndex;
		uniformBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uniformBinding.descriptorCount = 1;
		uniformBinding.stageFlags = stageFlags[i];
		uniformBinding.pImmutableSamplers = nullptr;
		layoutBindings.emplace_back(uniformBinding);

		bindingIndex++;

		uint32_t imageSize = material->Data((MShaderStage)i)->ImageSize();
		if (imageSize > 0)
		{
			VkDescriptorSetLayoutBinding imageBinding;
			imageBinding.binding = bindingIndex;
			imageBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageBinding.descriptorCount = imageSize;
			imageBinding.stageFlags = stageFlags[i];
			imageBinding.pImmutableSamplers = nullptr;
			layoutBindings.emplace_back(imageBinding);
		}
	}

	VK_STRUCT_CREATE(VkDescriptorSetLayoutCreateInfo, createInfo, VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO);
	createInfo.bindingCount = layoutBindings.size();
	createInfo.pBindings = layoutBindings.data();

	VK_RESULT_CHECK(vkCreateDescriptorSetLayout, Device(), &createInfo, nullptr, &layout);
	
	for (uint32_t i = 0; i < layouts.size(); ++i)
	{
		layouts[i] = layout;
	}
}


void VKDescriptorManager::AllocateSets(const VersionArray<VkDescriptorSetLayout>& layouts, 
	VersionArray<VkDescriptorSet>& sets)
{
	VkDevice device = Device();

	VkDescriptorSetAllocateInfo allocateInfo{};
	allocateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocateInfo.pNext = nullptr;
	allocateInfo.descriptorPool = _pool;
	allocateInfo.descriptorSetCount = layouts.size();
	allocateInfo.pSetLayouts = layouts.data();

	VK_RESULT_CHECK(vkAllocateDescriptorSets, device, &allocateInfo, sets.data());
}

void VKDescriptorManager::UpdateSets(VKUniform* uniform, 
	const VersionArray<VkDescriptorSet>& sets)
{
	MMaterial* material = uniform->Material();

	MVector<VkWriteDescriptorSet> writes;
	writes.reserve(kMShaderStageCount << 1);
	VkDescriptorBufferInfo bufferInfos[kMShaderStageCount];
	MVector<VkDescriptorImageInfo> imageInfos[kMShaderStageCount];
	uint32_t bindingIndex = 0;
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		VkWriteDescriptorSet bufferWrite{};
		bufferWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		bufferWrite.pNext = nullptr;
		bufferWrite.dstBinding = bindingIndex;
		bufferWrite.dstArrayElement = 0;
		bufferWrite.descriptorCount = 1;
		bufferWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		bufferWrite.pBufferInfo = &bufferInfos[i];
		writes.emplace_back(bufferWrite);
		bindingIndex++;

		uint32_t imageSize = material->Data((MShaderStage)i)->ImageSize();
		if (imageSize > 0)
		{
			VkWriteDescriptorSet imageWrite{};
			MVector<VkDescriptorImageInfo>& imageInfo = imageInfos[i];
			imageInfo.resize(imageSize);
			imageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			imageWrite.pNext = nullptr;
			imageWrite.dstBinding = bindingIndex;
			imageWrite.dstArrayElement = 0;
			imageWrite.descriptorCount = imageInfo.size();
			imageWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			imageWrite.pImageInfo = imageInfo.data();
			writes.emplace_back(imageWrite);
			bindingIndex++;
		}
	}

	for (int i = 0; i < MAX_PARALLEL_FRAME_VERSION; ++i)
	{
		for (uint32_t j = 0; j < kMShaderStageCount; ++j)
		{
			VKBuffer* buffer = uniform->Buffer(i, (MShaderStage)j);
			VkDescriptorBufferInfo& bufferInfo = bufferInfos[j];
			bufferInfo.buffer = buffer->Access();
			bufferInfo.offset = 0;
			bufferInfo.range = buffer->Size();


			MVector<VkDescriptorImageInfo>& imageInfo = imageInfos[j];
			if(imageInfo.size() > 0)
			{
				const MVector<VKImage*>& images = uniform->Material()->Data((MShaderStage)j)->AccessImage();

				for (uint32_t k = 0; k < images.size(); ++k)
				{
					VKImage* image = images[k];
					imageInfo[k].imageLayout = image->AccessDescriptorLayout();
					imageInfo[k].imageView = image->AccessView();
					imageInfo[k].sampler = image->AccessSampler();
				}
			}
		}
		for (VkWriteDescriptorSet& write : writes)
		{
			write.dstSet = sets[i];
		}
		vkUpdateDescriptorSets(Device(), writes.size(), writes.data(), 0, nullptr);
	}
}


MORISA_NAMESPACE_END
