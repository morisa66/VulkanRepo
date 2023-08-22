#ifndef __MATERIAL_DATA_H__
#define __MATERIAL_DATA_H__

#include "Core/GlobalConfig.h"
#include "Utils/MatrixUtils.h"
#include "Core/MVector.h"

#include <string>
#include "Core/MMap.h"

MORISA_NAMESPACE_BEGIN

class VKImage;
class MMaterialData
{
	friend class MMaterial;
public:
	MMaterialData();

	~MMaterialData();
	void AddFloat(const std::string& name, float value);
	void AddInt(const std::string& name, int value);
	void AddVec(const std::string& name, glm::vec4 value);
	void AddMat(const std::string& name, glm::mat4 value);
	void AddImage(const std::string& name, VKImage* value);
	void SetFloat(const std::string& name, float value);
	void SetInt(const std::string& name, int value);
	void SetVec(const std::string& name, glm::vec4 value);
	void SetMat(const std::string& name, glm::mat4 value);
	void SetImage(const std::string& name, VKImage* value);
	float GetFloat(const std::string& name);
	int GetInt(const std::string& name);
	const glm::vec4& GetVec(const std::string& name);
	const glm::mat4& GetMat(const std::string& name);
	VKImage* GetImage(const std::string& name);
	uint32_t DataSize();
	uint32_t ImageSize();
	uint32_t Hash();
	bool Dirty();
	void FlushData();
	void* AccessData();
	const MVector<VKImage*>& AccessImage();
private:
	typedef MUMap<std::string, uint32_t> IndexMap;
	typedef IndexMap::const_iterator IndexIter;

	void* _data;
	uint32_t _dataSize;
	bool _dirty;

	IndexMap _floatIndices;
	IndexMap _intIndices;
	IndexMap _vecIndices;
	IndexMap _matIndices;
	IndexMap _imageIndices;

	MVector<float> _floats;
	MVector<int> _ints;
	MVector<glm::vec4> _vecs;
	MVector<glm::mat4> _mats;
	MVector<VKImage*> _images;
};

MORISA_NAMESPACE_END

#endif

