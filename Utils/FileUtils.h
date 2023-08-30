#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include "Core/GlobalConfig.h"
#include "Core/MString.h"
#include "Core/MVector.h"
#include <fstream>

MORISA_NAMESPACE_BEGIN



// TODO
// Async read
class ScopeReader
{
public:
	ScopeReader(const char* path, MString& output);
	ScopeReader(const char* path, MVector<char>& output);
	~ScopeReader();
private:
	bool OpenFileStream(const char* path, std::ios::openmode mode);
	size_t Size();
	std::ifstream _stream;
};


MORISA_NAMESPACE_END

#endif
