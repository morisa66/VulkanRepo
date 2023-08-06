#ifndef __VK_PIPELINE_H__
#define __VK_PIPELINE_H__

#include "VKInclude.h"

#include <unordered_map>

MORISA_NAMESPACE_BEGIN

class VKGpuProgram;
class VKPipelineState;
class VKRenderPass;

struct VKPipelineKey
{
	VKGpuProgram* program;
	VKPipelineState* state;
	VKRenderPass* pass;
	VKPipelineKey()
	{
		memset(this, 0, sizeof(VKPipelineKey));
	}
};	

namespace detail
{
	struct VKPipelineKeyHash
	{
		std::size_t operator()(const VKPipelineKey& key) const noexcept
		{
			return reinterpret_cast<std::size_t>(&key);
		}
	};

	struct VKPipelineKeyEqual
	{
		bool operator()(const VKPipelineKey& lhs, const VKPipelineKey& rhs) const
		{
			return memcmp(&lhs, &rhs, sizeof(VKPipelineKey)) == 0;
		}
	};
}


// TODO
// NOT Support concurrent GET
typedef std::unordered_map<VKPipelineKey, VkPipeline, detail::VKPipelineKeyHash, detail::VKPipelineKeyEqual> PipelineCache;
typedef PipelineCache::iterator VKPipelineNode;

class VKPipelineCache
{
public:
	VKPipelineCache();
	~VKPipelineCache();
	VkPipeline GetCachePipeline(const VKPipelineKey& key);
private:
	PipelineCache _pipelineCache;
};

MORISA_NAMESPACE_END

#endif