#ifndef __LOG_UTILS_H__
#define __LOG_UTILS_H__

#include "Core/GlobalConfig.h"

#if ENABLE_LOG
#include <stdio.h>
#define MORISA_LOG(FORMAT, ...) printf(FORMAT, __VA_ARGS__)
#else
#define MORISA_LOG(FORMAT, ...)
#endif

#endif
