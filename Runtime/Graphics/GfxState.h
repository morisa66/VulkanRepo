#ifndef __GFX_STATE_H__
#define __GFX_STATE_H__

#include  "Core/GlobalConfig.h"


MORISA_NAMESPACE_BEGIN

enum MBlend
{
	kMBlendOneZero,
	kMBlendSrcAlpha,
	kMBlendAdd,
	kMBlendCount,
};

enum MShaderStage
{
	kMShaderStageVertex,
	kMShaderStageFragment,
	kMShaderStageCount,
};




MORISA_NAMESPACE_END

#endif
