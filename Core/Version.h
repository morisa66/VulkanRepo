#ifndef __VK_VERSION_H__
#define __VK_VERSION_H__

#include "GlobalConfig.h"

#include <stdint.h>
#include <array>


#define MAX_PARALLEL_FRAME_VERSION 2

MORISA_NAMESPACE_BEGIN

template<typename T>
using VersionArray = std::array<T, MAX_PARALLEL_FRAME_VERSION>;

class Version
{
public:
	Version();
	inline void Flush(uint32_t version) { _version = version; }
protected:
	uint32_t _version;
};

MORISA_NAMESPACE_END

#endif
