#ifndef __VK_UNIFORM_H__
#define __VK_UNIFORM_H__

#include "VKInclude.h"
#include "Core/GC.h"
#include "Core/Version.h"
#include "Utils/MatrixUtils.h"
#include "Runtime/Graphics/GfxState.h"

#include "Core/MVector.h"
#include "Core/MMap.h"
#include "Core/MString.h"

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
	inline VKDescriptorSet* DescriptorSet() { return _descriptorSets[0]; }
	inline VKDescriptorSet* DescriptorSet(uint32_t index) { return _descriptorSets[index]; }
private:
	VKUniform(MMaterial* material);
	VKUniform(MMaterial* material, const MMap<MString, MVector<VKImage*>>& images);
	void Update();
	void CreateDescriptorSet();
	void CreateDescriptorSets(const MMap<MString, MVector<VKImage*>>& images);
	void InitBuffer();
private:
	MMaterial* _material;
	VersionArray<VKBuffer*> _buffers[kMShaderStageCount];
	MVector<VKDescriptorSet*> _descriptorSets;
};

class VKUniformManager : public GC
{
public:
	VKUniformManager();
	VKUniform* CreateUniform(MMaterial* material);
	VKUniform* CreateUniform(MMaterial* material, const MMap<MString, MVector<VKImage*>>& images);
	void Update();
	void Flush(uint32_t version);
};

MORISA_NAMESPACE_END

#endif
