#ifndef __MATERIAL_DATA_H__
#define __MATERIAL_DATA_H__

#include "Core/GlobalConfig.h"
#include "Utils/MatrixUtils.h"
#include "Core/MVector.h"
#include "Core/MMap.h"
#include "Core/MString.h"

MORISA_NAMESPACE_BEGIN

extern const MString MainTex;
extern const MString ColorTex;
extern const MString DepthTex;

class VKImage;
class MMaterialData
{
	friend class MMaterial;
public:
	MMaterialData();

	~MMaterialData();
	void AddFloat(const MString& name, float value);
	void AddInt(const MString& name, int value);
	void AddVec(const MString& name, glm::vec4 value);
	void AddMat(const MString& name, glm::mat4 value);
	void AddImage(const MString& name, VKImage* value);
	void SetFloat(const MString& name, float value);
	void SetInt(const MString& name, int value);
	void SetVec(const MString& name, glm::vec4 value);
	void SetMat(const MString& name, glm::mat4 value);
	void SetImage(const MString& name, VKImage* value);
	float GetFloat(const MString& name);
	int GetInt(const MString& name);
	const glm::vec4& GetVec(const MString& name);
	const glm::mat4& GetMat(const MString& name);
	VKImage* GetImage(const MString& name);
	uint32_t DataSize();
	uint32_t ImageSize();
	uint32_t Hash();
	bool Dirty();
	void FlushData();
	void* AccessData();
	const MVector<VKImage*>& AccessImage();
private:
	typedef MUMap<MString, uint32_t> IndexMap;
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

