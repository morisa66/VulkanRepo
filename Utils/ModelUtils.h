#ifndef __MODEL_UTILS_H__
#define __MODEL_UTILS_H__

#include "Core/GlobalConfig.h"
#include "MatrixUtils.h"
#include "ImageUtils.h"
#include "Core/MVector.h"
#include "Core/MString.h"

MORISA_NAMESPACE_BEGIN


struct MMeshVertexData
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
};

struct MImageData
{
	MString path;
	MImage* image;
};

struct MMesh
{
	MVector<MMeshVertexData> vertices;
	MVector<uint32_t> indices;
	MVector<uint32_t> imageIndices;
};

class MModel
{
	friend MModel* LoadModel(const char* path);
public:
	~MModel();
	const MVector<MMesh*>& Meshs() { return _meshs; }
	const MVector<MImageData>& ImageDatas() { return _imageDatas; }
private:
	MModel();
	MVector<MMesh*> _meshs;
	MVector<MImageData> _imageDatas;
};

MModel* LoadModel(const char* path);

extern MMesh mPlaneMesh;
extern MMesh mQuadMesh;
extern MMesh mCubeMesh;

MORISA_NAMESPACE_END


#endif