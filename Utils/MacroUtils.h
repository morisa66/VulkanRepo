#ifndef __MACRO_UTILS_H__
#define __MACRO_UTILS_H__

#include <malloc.h>

#define VK_STRUCT_INIT(Struct, Type)\
	Struct.sType = Type;\
	Struct.pNext = nullptr;\
	Struct.flags = 0;

#define VK_STRUCT_CREATE(Name, Struct, Type)\
	Name Struct{};\
	VK_STRUCT_INIT(Struct, Type)


#define VK_RESULT_CHECK(FUNC, ...)\
{\
VkResult __RESULT__ = FUNC(__VA_ARGS__);\
if(__RESULT__ != VK_SUCCESS)\
{\
	MORISA_LOG("%s ERROR %d\n", #FUNC, __RESULT__);\
	MORISA_LOG("File name :%s\n", __FILE__);\
	MORISA_LOG("File line :%d\n", __LINE__);\
}\
}

#define VK_SUCCESS_CALL(FUNC, CALL, ...)\
VK_RESULT_CHECK(FUNC, __VA_ARGS__)\
else\
{\
	CALL;\
}

#define VK_VALID_DESTROY(HANDLE, FUNC, ...)\
if(HANDLE != VK_NULL_HANDLE)\
{\
	FUNC(__VA_ARGS__);\
}

#define MORISA_ALLOC(SIZE)\
malloc(SIZE);

#define MORISA_FREE(PTR)\
if(PTR != nullptr)\
{\
	free(PTR);\
}

#define MORISA_DELETE(PTR)\
if(PTR != nullptr)\
{\
	delete PTR;\
}

#define MORISA_ARRAY_DELETE(PTR)\
if(PTR != nullptr)\
{\
	delete[] PTR;\
	PTR = nullptr;\
}

#define MORISA_NEW(NAME, ...)\
new NAME(__VA_ARGS__)


#define DESTROY_POOL(TYPE, POOL, DESTROYFUNC, ...)\
while (!POOL.empty())\
{\
	TYPE TYPE##HANDLE = POOL.front();\
	VK_VALID_DESTROY(TYPE##HANDLE, DESTROYFUNC, __VA_ARGS__);\
	POOL.pop();\
}

#endif
