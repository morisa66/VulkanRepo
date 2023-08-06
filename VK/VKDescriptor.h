#ifndef __VK_DESCRIPTOR_H__
#define __VK_DESCRIPTOR_H__

#include "Core/Version.h"
#include "Core/GC.h"
#include "VKInclude.h"

#include <array>

MORISA_NAMESPACE_BEGIN

class VKUniform;
class VKDescriptorManager;
class MMaterial;

class VKDescriptorSet : public GCNode, public Version
{
	friend class VKDescriptorManager;
public:
	virtual ~VKDescriptorSet();
	inline VkDescriptorSetLayout Layout() { return _layouts[0]; }
	inline const VkDescriptorSet* SetPtr() { return &_sets[_version]; }
	inline VkDescriptorSet Set() { return _sets[_version]; }
private:
	VKDescriptorSet();
	VersionArray<VkDescriptorSetLayout> _layouts;
	VersionArray<VkDescriptorSet> _sets;
};

class VKDescriptorManager : public GC	
{
	friend class VKDescriptor;
public:
	VKDescriptorManager();
	~VKDescriptorManager();
	VKDescriptorSet* CreateDescriptorSet(VKUniform* uniform);
	void Update(VKUniform* uniform);
private:
	void CreatePool();
	void CreateLayout(MMaterial* material,
		VersionArray<VkDescriptorSetLayout>& layouts);
	void AllocateSets(const VersionArray<VkDescriptorSetLayout>& layouts,
		 VersionArray<VkDescriptorSet>& sets);
	void UpdateSets(VKUniform* uniform, const VersionArray<VkDescriptorSet>& sets);
private:
	VkDescriptorPool _pool;
};


MORISA_NAMESPACE_END


#endif