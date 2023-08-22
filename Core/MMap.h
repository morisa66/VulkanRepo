#ifndef __MMAP_H__
#define __MMAP_H__

#include "GlobalConfig.h"
#include <map>
#include <unordered_map>


MORISA_NAMESPACE_BEGIN

template <typename K, typename V>
using MMap = std::map<K, V>;

template <typename K, typename V>
using MUMap = std::unordered_map<K, V>;

template <typename K, typename V, typename Hash, typename Equal>
using MUMapCache = std::unordered_map<K, V, Hash, Equal>;

MORISA_NAMESPACE_END


#endif