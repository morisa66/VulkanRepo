#ifndef __RENDER_NODE_H__
#define __RENDER_NODE_H__

#include "Core/GlobalConfig.h"

MORISA_NAMESPACE_BEGIN

class MMaterial;
class VKMesh;
class VKUniform;
class VKPipelineState;

struct MRenderNode
{
	MMaterial* material;
	VKMesh* mesh;
	VKUniform* uniform;
	VKPipelineState* state;

	MRenderNode();
	~MRenderNode();
};


MORISA_NAMESPACE_END

#endif 

