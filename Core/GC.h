#ifndef __VK_GC_H__
#define __VK_GC_H__

#include "GlobalConfig.h"

#include "Core/MMap.h"
#include "Core/MVector.h"


MORISA_NAMESPACE_BEGIN

class GCNode
{
	friend class GC;
public:
	GCNode();
	virtual ~GCNode() {}
protected:
	uint32_t _id;
	uint32_t _inGC : 1;
	uint32_t _inPersistence : 1;
	uint32_t _inDelay : 1;
};

// TODO Concurrently support
class GC
{
public:
	virtual ~GC();

	void Destroy(uint64_t fence);

	void DelayDestroy()
	{
		Destroy(_delayPool);
	}

	void PushDelayDestroy(GCNode* node);

protected:
	void PushGC(uint64_t fence, GCNode* node);

	void PushPersistence(GCNode* node);
	MUMap<uint64_t, MVector<GCNode*>> _GCPool;
	typedef MUMap<uint32_t, GCNode*> POOL;
	// uint64_t is a fence
	POOL _persistencePool;
	POOL _delayPool;
	void Destroy(POOL& pool);
};

MORISA_NAMESPACE_END

#endif