#ifndef __VK_GPU_PROGRAM_H__
#define __VK_GPU_PROGRAM_H__

#include "VKInclude.h"

#include <string>

MORISA_NAMESPACE_BEGIN

enum VKShaderType
{
	VKShaderTypeVS,
	VKShaderTypeFS,
	VKShaderTypeCount,
};

class VKGpuProgram
{
public:
	VKGpuProgram(const char* shaderPathVS, const char* shaderPathFS);
	~VKGpuProgram();
	inline const VkPipelineShaderStageCreateInfo* GetStageInfo() { return _shaderStageCreateInfo; }
private:
	void InitDefault();
	void CreateShaderModule(const char* shaderPathVS, const char* shaderPathFS);
	VkShaderModule CreateShaderMoudle(const std::string& spirvPath);
	void ConfigureShaderStageInfo();
private:
	VkShaderModule _shaderModules[VKShaderTypeCount];
	VkPipelineShaderStageCreateInfo _shaderStageCreateInfo[VKShaderTypeCount];
};

MORISA_NAMESPACE_END

#endif
