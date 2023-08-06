#include "GC.h"
#include "Utils/MacroUtils.h"

#include <atomic>

MORISA_NAMESPACE_BEGIN

static std::atomic_uint32_t IDGenerator = 0;

GCNode::GCNode() :
	_id(IDGenerator.fetch_add(1, std::memory_order_acquire))
	, _inGC(0)
	, _inPersistence(0)
	, _inDelay(0)
{

}

GC::~GC()
{
	Destroy(_delayPool);

	Destroy(_persistencePool);

	for (auto it = _GCPool.begin(); it != _GCPool.end(); ++it)
	{
		for (GCNode* node : it->second)
		{
			MORISA_DELETE(node);
		}
	}
}

void GC::Destroy(uint64_t fence)
{
	auto nodes = _GCPool.find(fence);
	if (nodes != _GCPool.end())
	{
		for (GCNode* node : nodes->second)
		{
			MORISA_DELETE(node);
		}
		_GCPool.erase(nodes);
	}
}

void GC::PushDelayDestroy(GCNode* node)
{
	if (node == nullptr)
	{
		return;
	}
	if (node->_inGC || node->_inDelay)
	{
		return;
	}
	_delayPool.emplace(std::make_pair(node->_id, node));
	node->_inDelay = 1;
	if (node->_inPersistence)
	{
		_persistencePool.erase(node->_id);
		node->_inPersistence = 0;
	}
}

void GC::PushGC(uint64_t fence, GCNode* node)
{
	auto nodes = _GCPool.find(fence);
	if (nodes != _GCPool.end())
	{
		nodes->second.emplace_back(node);
	}
	else
	{
		_GCPool.emplace(std::make_pair(fence, std::vector<GCNode*>{node}));
	}

	node->_inGC = 1;
}

void GC::PushPersistence(GCNode* node)
{
	_persistencePool.emplace(std::make_pair(node->_id, node));
	node->_inPersistence = 1;
}

void GC::Destroy(POOL& pool)
{
	if (!pool.empty())
	{
		for (POOL::iterator it = pool.begin(); it != pool.end(); ++it)
		{
			GCNode* node = it->second;
			MORISA_DELETE(node);
		}
		pool.clear();
	}
}



MORISA_NAMESPACE_END
