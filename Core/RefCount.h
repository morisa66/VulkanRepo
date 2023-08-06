#ifndef __REF_COUNT_H__
#define __REF_COUNT_H__

#include "GlobalConfig.h"

#include <atomic>


MORISA_NAMESPACE_BEGIN

class RefCount
{
public:
	virtual ~RefCount();
	void AddRef();
	bool SubRef();
protected:
	RefCount();
private:	
	std::atomic_uint32_t _refCount;
};

MORISA_NAMESPACE_END

#endif