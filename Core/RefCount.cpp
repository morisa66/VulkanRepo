#include "RefCount.h"

MORISA_NAMESPACE_BEGIN

RefCount::RefCount():
	_refCount(0)
{
}

RefCount::~RefCount()
{
}

void RefCount::AddRef()
{
	_refCount.fetch_add(1, std::memory_order_relaxed);
}

bool RefCount::SubRef()
{
	return _refCount.fetch_sub(1, std::memory_order_acq_rel) == 1;
}

MORISA_NAMESPACE_END


