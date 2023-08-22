#ifndef __MODEL_UTILS_H__
#define __MODEL_UTILS_H__

#include "Core/GlobalConfig.h"
#include "MatrixUtils.h"

#include "Core/MVector.h"

MORISA_NAMESPACE_BEGIN


struct MMeshVertexData
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 uv;
};

struct MMesh
{
	MVector<MMeshVertexData> vertices;
	MVector<uint32_t> indices;
};

class MModel
{
	friend MModel* LoadModel(const char* path);
public:
	~MModel();
	const MVector<MMesh*>& Meshs() { return _meshs; }
private:
	MModel();
	MVector<MMesh*> _meshs;
};

MModel* LoadModel(const char* path);

MORISA_NAMESPACE_END


#endif