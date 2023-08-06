#include "Shader.h"
#include "VK/VKGpuProgram.h"

MORISA_NAMESPACE_BEGIN

MShader::MShader(const char* pathVS, const char* pathFS):
	_blend(kMBlendOneZero)
{
	_pathNames[kMShaderStageVertex] = pathVS;
	_pathNames[kMShaderStageFragment] = pathFS;
	_program = MORISA_NEW(VKGpuProgram, pathVS, pathFS);
}

MShader::~MShader()
{
	MORISA_DELETE(_program);
}

MORISA_NAMESPACE_END


