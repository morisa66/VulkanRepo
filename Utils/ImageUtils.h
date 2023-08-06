#ifndef __M_IMAGE_H__
#define __M_IMAGE_H__

#include "Core/GlobalConfig.h"

#include <stdint.h>

MORISA_NAMESPACE_BEGIN


class MImage
{
public:
	MImage(const char* path);
	~MImage();
	// TODO maybe overflow
	inline uint64_t Size() { return _size; }
	inline int Width() { return _width; }
	inline int Height() { return _height; }
	inline int Channel() { return _channel; }
	inline void* Access() { return _data; }
	uint32_t MipLevels();
	void FreeImageReadMemory();
private:
	void* _data;
	int _width;
	int _height;
	int _channel;
	uint64_t _size;
};

MORISA_NAMESPACE_END

#endif