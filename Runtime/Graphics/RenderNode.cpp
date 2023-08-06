#include "RenderNode.h"
#include "Material.h"
#include "VK/VKPipelineState.h"
#include "Utils/MacroUtils.h"

MORISA_NAMESPACE_BEGIN

MRenderNode::MRenderNode():
	material(nullptr)
	, mesh(nullptr)
	, uniform(nullptr)
	, state(nullptr)
{
}

MRenderNode::~MRenderNode()
{
	MORISA_DELETE(material);
	MORISA_DELETE(state);
}

MORISA_NAMESPACE_END

