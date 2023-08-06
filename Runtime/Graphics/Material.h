#ifndef __MATERIAL_H__
#define __MATERIAL_H__

#include "Core/GlobalConfig.h"
#include "MaterialData.h"
#include "Shader.h"

MORISA_NAMESPACE_BEGIN

class MMaterial
{
public:
	MMaterial(MShader* shader);
	MMaterial();
	~MMaterial();
	
	void FlushData();

	MShader* Shader() { return _shader; }
	MMaterialData* Data(MShaderStage stage) { return _datas[stage]; }

	void SetFloat(const std::string& name, float value);
	void SetInt(const std::string& name, int value);
	void SetVec(const std::string& name, glm::vec4 value);
	void SetMat(const std::string& name, glm::mat4 value);
	void SetImage(const std::string& name, VKImage* value);
private:
	MShader* _shader;
	MMaterialData* _datas[kMShaderStageCount];
};

MORISA_NAMESPACE_END


#endif