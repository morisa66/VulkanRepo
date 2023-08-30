#include "GlobalConfig.h"

static GlobalConfig _globalConfig;
struct _Dummy
{
	_Dummy()
	{
		_globalConfig.defaultWindowWidth = 1200;
		_globalConfig.defaultWindowHeight = 720;
		_globalConfig.enableValidationLayer = true;
		_globalConfig.hideCursor = false;

		_globalConfig.resourcesRootPath = "../Resources";
		_globalConfig.shaderRootPath = "../Shader";
		_globalConfig.compilerPath = "../Third/Bin/glslc.exe";
		_globalConfig.spirvSuffix = ".spv";
		_globalConfig.shadeSuffixVS = ".vert";
		_globalConfig.shadeSuffixFS = ".frag";

		_globalConfig.useMSAA = true;

		_globalConfig.shadowMapWidth = 2048;
		_globalConfig.shadowMapHeight = 2048;
	}
};
_Dummy _dummy;

const GlobalConfig& globalConfig = _globalConfig;