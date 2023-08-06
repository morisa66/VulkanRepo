#include "MaterialData.h"
#include "Utils/MacroUtils.h"

MORISA_NAMESPACE_BEGIN

#define ADD_VALUE(INDICES, VALUES, NAME, VALUE)\
IndexIter it = INDICES.find(NAME);\
_dirty = true;\
if (it == INDICES.cend())\
{\
	INDICES.emplace(name, VALUES.size());\
	VALUES.emplace_back(VALUE);\
}\
else\
{\
	VALUES[it->second] = VALUE;\
}

#define SET_VALUE(INDICES, VALUES, NAME, VALUE)\
_dirty = true;\
IndexIter it = INDICES.find(NAME);\
if (it != INDICES.cend())\
{\
	VALUES[it->second] = VALUE;\
}

template <typename T>
uint32_t VectorSize(const std::vector<T>& v)
{
	return sizeof(T) * v.size();
}

MMaterialData::MMaterialData():
	_data(nullptr)
	, _dataSize(16)
	, _dirty(true)
{
	_data = MORISA_ALLOC(_dataSize);
	memset(_data, 0, _dataSize);
}

MMaterialData::~MMaterialData()
{
}

void MMaterialData::AddFloat(const std::string& name, float value)
{
	ADD_VALUE(_floatIndices, _floats, name, value);
}

void MMaterialData::AddInt(const std::string& name, int value)
{
	ADD_VALUE(_intIndices, _ints, name, value);
}

void MMaterialData::AddVec(const std::string& name, glm::vec4 value)
{
	ADD_VALUE(_vecIndices, _vecs, name, value);
}

void MMaterialData::AddMat(const std::string& name, glm::mat4 value)
{
	ADD_VALUE(_matIndices, _mats, name, value);
}

void MMaterialData::AddImage(const std::string& name, VKImage* value)
{
	ADD_VALUE(_imageIndices, _images, name, value);
}


void MMaterialData::SetFloat(const std::string& name, float value)
{
	SET_VALUE(_floatIndices, _floats, name, value);
}

void MMaterialData::SetInt(const std::string& name, int value)
{
	SET_VALUE(_intIndices, _ints, name, value);
}

void MMaterialData::SetVec(const std::string& name, glm::vec4 value)
{
	SET_VALUE(_vecIndices, _vecs, name, value);
}

void MMaterialData::SetMat(const std::string& name, glm::mat4 value)
{
	SET_VALUE(_matIndices, _mats, name, value);
}

void MMaterialData::SetImage(const std::string& name, VKImage* value)
{
	SET_VALUE(_imageIndices, _images, name, value);
}


uint32_t MMaterialData::DataSize()
{
	return _dataSize;
}

uint32_t MMaterialData::ImageSize()
{
	return _images.size();
}


uint32_t MMaterialData::Hash()
{
	// EACH MAX COUNT 1 << 6
	return _floats.size()
		+ (_ints.size() << 6)
		+ (_vecs.size() << 12)
		+ (_mats.size() << 18)
		+ (_images.size() << 24);
}

bool MMaterialData::Dirty()
{
	return _dirty;
}

void MMaterialData::FlushData()
{
	if (!_dirty)
	{
		return;
	}
	_dirty = false;

	// maybe cause crash
	uint32_t  currentSize = VectorSize(_floats) + VectorSize(_ints) + VectorSize(_vecs) + VectorSize(_mats);
	if (_dataSize < currentSize)
	{
		_dataSize = currentSize;
		MORISA_FREE(_data);
		_data = MORISA_ALLOC(currentSize);
	}

	// Mat -> vec -> float -> int
	uint32_t offset = 0;
	uint32_t size = VectorSize(_mats);
	if (size > 0)
	{
		memcpy(_data, _mats.data(), size);
	}

	offset += size;
	size = VectorSize(_vecs);
	if (size > 0)
	{
		memcpy((uint8_t*)_data + offset, _vecs.data(), size);
	}

	offset += size;
	size = VectorSize(_floats);
	if (size > 0)
	{
		memcpy((uint8_t*)_data + offset, _floats.data(), size);
	}

	offset += size;
	size = VectorSize(_ints);
	if (size > 0)
	{
		memcpy((uint8_t*)_data + offset, _ints.data(), size);
	}
}

void* MMaterialData::AccessData()
{
	return _data;
}

const std::vector<VKImage*>& MMaterialData::AccessImage()
{
	return _images;
}

MORISA_NAMESPACE_END
