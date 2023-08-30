#include "VKUniform.h"
#include "VKContext.h"
#include "VKBuffer.h"
#include "VKDescriptor.h"
#include "VKImage.h"
#include "Runtime/Graphics/Material.h"

#include <chrono>

MORISA_NAMESPACE_BEGIN

#define MAX_IMAGE_SAMPLER 8

VKUniform::VKUniform(MMaterial* material):
	_material(material)
{
	InitBuffer();
	CreateDescriptorSet();
}

VKUniform::VKUniform(MMaterial* material, const MMap<MString, MVector<VKImage*>>& images):
	_material(material)
{
	InitBuffer();
	CreateDescriptorSets(images);
}


VKUniform::~VKUniform()
{
}

void VKUniform::Update()
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		MMaterialData* data = _material->Data((MShaderStage)i);
		if (data->Dirty())
		{
			data->FlushData();
			memcpy(Buffer((MShaderStage)i)->AccessData(), data->AccessData(), Buffer((MShaderStage)i)->Size());
		}
	}
}

void VKUniform::CreateDescriptorSet()
{
	_descriptorSets.resize(1);
	_descriptorSets[0] = Context()->DescriptorManager()->CreateDescriptorSet(this);
}

void VKUniform::CreateDescriptorSets(const MMap<MString, MVector<VKImage*>>& images)
{
	MMap<MString, MVector<VKImage*>>::const_iterator it = images.find(MainTex);
	if (it == images.cend() || it->second.size() < 2)
	{
		_material->SetImage(MainTex, it->second[0]);
		CreateDescriptorSet();
	}
	else
	{
		uint32_t size = it->second.size();
		_descriptorSets.resize(size);
		for (uint32_t i = 0; i < size; ++i)
		{
			_material->SetImage(MainTex, it->second[i]);
			_descriptorSets[i] = Context()->DescriptorManager()->CreateDescriptorSet(this);
		}
	}
}

void VKUniform::InitBuffer()
{
	_material->FlushData();

	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		MMaterialData* data = _material->Data((MShaderStage)i);
		for (uint32_t j = 0; j < MAX_PARALLEL_FRAME_VERSION; j++)
		{
			_buffers[i][j] = Context()->BufferManager()->CreatePersistenceBuffer(
				data->DataSize(),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				false,
				data->AccessData());
		}
	}
}


VKUniformManager::VKUniformManager()
{
}

VKUniform* VKUniformManager::CreateUniform(MMaterial* material)
{
	VKUniform* uniform = MORISA_NEW(VKUniform, material);
	PushPersistence(uniform);
	return uniform;
}

VKUniform* VKUniformManager::CreateUniform(MMaterial* material, const MMap<MString, MVector<VKImage*>>& images)
{
	VKUniform* uniform = MORISA_NEW(VKUniform, material, images);
	PushPersistence(uniform);
	return uniform;
}

void VKUniformManager::Update()
{
	for (POOL::iterator it = _persistencePool.begin(); it != _persistencePool.end(); ++it)
	{
		dynamic_cast<VKUniform*>(it->second)->Update();
	}
}

void VKUniformManager::Flush(uint32_t version)
{
	for (POOL::iterator it = _persistencePool.begin(); it != _persistencePool.end(); ++it)
	{
		VKUniform* uniform = dynamic_cast<VKUniform*>(it->second);
		uniform->Flush(version);
		uniform->DescriptorSet()->Flush(version);
	}
}

MORISA_NAMESPACE_END


