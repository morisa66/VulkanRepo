#include "VKGpuProgram.h"
#include "VKContext.h"
#include "Utils/FileUtils.h"

#include <windows.h>

MORISA_NAMESPACE_BEGIN


static VKShaderType GetShaderType(const MString& fullPath)
{
    const int findPos = fullPath.size() - 5;
    if (fullPath.rfind(globalConfig.shadeSuffixVS) == findPos)
    {
        return VKShaderTypeVS;
    }
    if (fullPath.rfind(globalConfig.shadeSuffixFS) == findPos)
    {
        return VKShaderTypeFS;
    }
    return VKShaderTypeCount;
}


static MString CompileShader(const char* shaderPath)
{
    MString appPath = globalConfig.compilerPath;
    MString fullPath = globalConfig.shaderRootPath;
    fullPath.append(shaderPath);

    MString fullPathSpirv = globalConfig.shaderRootPath;
    fullPathSpirv.append("/SPIRV")
        .append(fullPath.substr(fullPath.rfind('/')))
        .append(globalConfig.spirvSuffix);
    VKShaderType shaderType = GetShaderType(fullPath);
    if (shaderType == VKShaderTypeCount)
    {
        MORISA_LOG("Unsupported shader %s suffix\n", fullPath.c_str());
        return "";
    }

    // Need space
	MString cmd = " ";

    cmd.append(fullPath);
    cmd.append(" -o ");
    cmd.append(fullPathSpirv);

    std::wstring wAppPath(appPath.begin(), appPath.end());
    std::wstring wCmd(cmd.begin(), cmd.end());
    STARTUPINFO sInfo{};
    PROCESS_INFORMATION pInfo{};

    if (!CreateProcess(wAppPath.c_str(), const_cast<LPWSTR>(wCmd.c_str()), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &sInfo, &pInfo))
    {
        MORISA_LOG("CreateProcess failed (%d).\n", GetLastError());
    }

    WaitForSingleObject(pInfo.hProcess, INFINITE);

    CloseHandle(pInfo.hProcess);
    CloseHandle(pInfo.hThread);

    return fullPathSpirv;
}

VKGpuProgram::VKGpuProgram( const char* shaderPathVS, const char* shaderPathFS)
{
    InitDefault();
    
    CreateShaderModule(shaderPathVS, shaderPathFS);

    ConfigureShaderStageInfo();
}

VKGpuProgram::~VKGpuProgram()
{
    for (int i = 0; i < VKShaderTypeCount; ++i)
    {
        VK_VALID_DESTROY(_shaderModules[i], vkDestroyShaderModule, Device(), _shaderModules[i], nullptr);
    }
}

void VKGpuProgram::InitDefault()
{
    for (int i = 0; i < VKShaderTypeCount; ++i)
    {
        _shaderModules[i] = VK_NULL_HANDLE;
        _shaderStageCreateInfo[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        _shaderStageCreateInfo[i].pNext = nullptr;
        _shaderStageCreateInfo[i].flags = 0;
        _shaderStageCreateInfo[i].pName = "main";
        _shaderStageCreateInfo[i].module = VK_NULL_HANDLE;
        _shaderStageCreateInfo[i].pSpecializationInfo = nullptr;
    }
}

void VKGpuProgram::CreateShaderModule(const char* shaderPathVS, const char* shaderPathFS)
{
    MString spirvVSPath = CompileShader(shaderPathVS);
    MString spirvFSPath = CompileShader(shaderPathFS);
    _shaderModules[VKShaderTypeVS] = CreateShaderMoudle(spirvVSPath);
    _shaderModules[VKShaderTypeFS] = CreateShaderMoudle(spirvFSPath);
}

VkShaderModule VKGpuProgram::CreateShaderMoudle(const MString& spirvPath)
{
    MVector<char> spirv;
    {
        ScopeReader spirvVSReader(spirvPath.c_str(), spirv);
    }
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VK_STRUCT_CREATE(VkShaderModuleCreateInfo, shaderModuleCreateInfo, VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
    shaderModuleCreateInfo.codeSize = spirv.size();
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(spirv.data());

    VK_RESULT_CHECK(vkCreateShaderModule, Device(), &shaderModuleCreateInfo, nullptr, &shaderModule);

    return shaderModule;
}

void VKGpuProgram::ConfigureShaderStageInfo()
{
    static const VkShaderStageFlagBits stages[VKShaderTypeCount] =
    {
        VK_SHADER_STAGE_VERTEX_BIT,
        VK_SHADER_STAGE_FRAGMENT_BIT,
    };

    for (int i = 0; i < VKShaderTypeCount; ++i)
    {
        _shaderStageCreateInfo[i].module = _shaderModules[i];
        _shaderStageCreateInfo[i].stage = stages[i];
    }
}

MORISA_NAMESPACE_END


