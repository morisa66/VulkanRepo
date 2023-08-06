#ifndef __MATERIAL_DATA_H__
#define __MATERIAL_DATA_H__

#include "Core/GlobalConfig.h"
#include "Utils/MatrixUtils.h"

#include <string>
#include <unordered_map>

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
	uint32_t DataSize();
	uint32_t ImageSize();
	uint32_t Hash();
	bool Dirty();
	void FlushData();
	void* AccessData();
	const std::vector<VKImage*>& AccessImage();
private:
	typedef std::unordered_map<std::string, uint32_t> IndexMap;
	typedef IndexMap::const_iterator IndexIter;

	void* _data;
	uint32_t _dataSize;
	bool _dirty;

	IndexMap _floatIndices;
	IndexMap _intIndices;
	IndexMap _vecIndices;
	IndexMap _matIndices;
	IndexMap _imageIndices;

	std::vector<float> _floats;
	std::vector<int> _ints;
	std::vector<glm::vec4> _vecs;
	std::vector<glm::mat4> _mats;
	std::vector<VKImage*> _images;
};

MORISA_NAMESPACE_END

#endif

