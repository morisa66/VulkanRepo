#ifndef __VK_TOOLS_H__
#define __VK_TOOLS_H__

#include "VKInclude.h"


MORISA_NAMESPACE_BEGIN

struct VKCheckSafetyDelete
{
	static VkDevice device;
	const bool operator()(uint64_t fence)
	{
		return vkGetFenceStatus(device, (VkFence)fence) == VK_SUCCESS;
	}
};


MORISA_NAMESPACE_END


#endif