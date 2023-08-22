#ifndef __GLOBAL_CONGIG_H__
#define __GLOBAL_CONGIG_H__

#define MORISA_NAMESPACE_BEGIN namespace morisa{
#define MORISA_NAMESPACE_END };

#define ENABLE_LOG 1

struct GlobalConfig
{
	int defaultWindowWidth;
	int defaultWindowHeight;
	bool enableValidationLayer;
	bool hideCursor;

	const char* resourcesRootPath;
	const char* shaderRootPath;
	const char* compilerPath;
	const char* spirvSuffix;
	const char* shadeSuffixVS;
	const char* shadeSuffixFS;

	bool useMSAA;

	int shadowMapWidth;
	int shadowMapHeight;
};

extern const GlobalConfig& globalConfig;

#endif
