#include "Core/GlobalConfig.h"

MORISA_NAMESPACE_BEGIN

template<typename T>
inline T Max(T a, T b) { return a > b ? a : b; }


template<typename T>
inline T Min(T a, T b) { return a < b ? a : b; }


MORISA_NAMESPACE_END
