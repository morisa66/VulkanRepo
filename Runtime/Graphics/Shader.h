#ifndef __SHADER_H__
#define __SHADER_H__

#include "Core/GlobalConfig.h"
#include "Core/RefCount.h"
#include "GfxState.h"

#include <string>

MORISA_NAMESPACE_BEGIN

class VKGpuProgram;


class MShader : public RefCount
{
public:
	MShader(const char* pathVS, const char* pathFS);
	virtual ~MShader();
	VKGpuProgram* Program() { return _program; }
private:
	MBlend _blend;
	std::string _pathNames[kMShaderStageCount];
	VKGpuProgram* _program;
};

MORISA_NAMESPACE_END


#endif