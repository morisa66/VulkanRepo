#include "FileUtils.h"
#include "LogUtils.h"

MORISA_NAMESPACE_BEGIN

ScopeReader::ScopeReader(const char* path, MString& output)
{
	if (OpenFileStream(path, std::ios::in))
	{
		output.clear();
		size_t size = Size();
		output.resize(size);
		_stream.read(const_cast<char*>(output.data()), size);
	}
}

ScopeReader::ScopeReader(const char* path, MVector<char>& output)
{
	if (OpenFileStream(path, std::ios::in | std::ios::binary))
	{
		output.clear();
		size_t size = Size();
		output.resize(size);
		_stream.read(output.data(), size);
	}
}

ScopeReader::~ScopeReader()
{
	if (_stream.is_open())
	{
		_stream.close();
	}
}

bool ScopeReader::OpenFileStream(const char* path, std::ios::openmode mode)
{
	_stream.open(path, mode);
	if (!_stream.is_open())
	{
		MORISA_LOG("Read file %s ERROR\n", path);
		return false;
	}
	return true;
}

size_t ScopeReader::Size()
{
	_stream.seekg(0, std::ios::end);
	size_t size = _stream.tellg();
	_stream.seekg(0);
	return size;
}


MORISA_NAMESPACE_END