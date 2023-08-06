#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include "Core/GlobalConfig.h"

#include <string>
#include <fstream>
#include <vector>

MORISA_NAMESPACE_BEGIN



// TODO
// Async read
class ScopeReader
{
public:
	ScopeReader(const char* path, std::string& output);
	ScopeReader(const char* path, std::vector<char>& output);
	~ScopeReader();
private:
	bool OpenFileStream(const char* path, std::ios::openmode mode);
	size_t Size();
	std::ifstream _stream;
};


MORISA_NAMESPACE_END

#endif
