#include "Material.h"
#include "Shader.h"
#include "Utils/MacroUtils.h"

MORISA_NAMESPACE_BEGIN

MMaterial::MMaterial(MShader* shader):
	_shader(shader)
{
	_shader->AddRef();
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i] = MORISA_NEW(MMaterialData);
	}
}

MMaterial::MMaterial():
	_shader(nullptr)
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i] = MORISA_NEW(MMaterialData);
	}
}

MMaterial::~MMaterial()
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		MORISA_DELETE(_datas[i]);
	}
	if(_shader != nullptr && _shader->SubRef())
	{
		MORISA_DELETE(_shader);
	}
}

void MMaterial::FlushData()
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i]->FlushData();
	}
}

void MMaterial::SetFloat(const std::string& name, float value)
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i]->SetFloat(name, value);
	}
}

void MMaterial::SetInt(const std::string& name, int value)
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i]->SetInt(name, value);
	}
}

void MMaterial::SetVec(const std::string& name, glm::vec4 value)
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i]->SetVec(name, value);
	}
}

void MMaterial::SetMat(const std::string& name, glm::mat4 value)
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i]->SetMat(name, value);
	}
}

void MMaterial::SetImage(const std::string& name, VKImage* value)
{
	for (uint32_t i = 0; i < kMShaderStageCount; ++i)
	{
		_datas[i]->SetImage(name, value);
	}
}


MORISA_NAMESPACE_END

