#ifndef __MODEL_UTILS_H__
#define __MODEL_UTILS_H__

#include "Core/GlobalConfig.h"
#include "MatrixUtils.h"

#include <vector>

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
	std::vector<MMeshVertexData> vertices;
	std::vector<uint32_t> indices;
};

class MModel
{
	friend MModel* LoadModel(const char* path);
public:
	~MModel();
	const std::vector<MMesh*>& Meshs() { return _meshs; }
private:
	MModel();
	std::vector<MMesh*> _meshs;
};

MModel* LoadModel(const char* path);

MORISA_NAMESPACE_END


#endif