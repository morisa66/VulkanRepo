#ifndef __VK_UNIFORM_H__
#define __VK_UNIFORM_H__

#include "VKInclude.h"
#include "Core/GC.h"
#include "Core/Version.h"
#include "Utils/MatrixUtils.h"
#include "Runtime/Graphics/GfxState.h"

#include "Core/MVector.h"

MORISA_NAMESPACE_BEGIN

class VKBuffer;
class VKImage;
class VKDescriptorSet;
class VKDescriptorManager;
class MMaterial;

class VKUniformManager;

class VKUniform : public GCNode, public Version
{
	friend VKUniformManager;
public:
	virtual ~VKUniform();
	inline VKBuffer* Buffer(uint32_t version, MShaderStage stage) { return _buffers[stage][version]; }
	inline VKBuffer* Buffer(MShaderStage stage) { return _buffers[stage][_version]; }
	inline MMaterial* Material() { return _material; };
	inline VKDescriptorSet* DescriptorSet() { return _descriptorSet; }
private:
	VKUniform(MMaterial* material);
	void Update();
	void CreateDescriptorSet();
	void InitBuffer();
private:
	MMaterial* _material;
	VersionArray<VKBuffer*> _buffers[kMShaderStageCount];
	VKDescriptorSet* _descriptorSet;
};

class VKUniformManager : public GC
{
public:
	VKUniformManager();
	VKUniform* CreateUniform(MMaterial* material);
	void Update();
	void Flush(uint32_t version);
};

MORISA_NAMESPACE_END

#endif
