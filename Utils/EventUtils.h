#ifndef __EVENT_UTILS_H__
#define __EVENT_UTILS_H__

#include "Core/GlobalConfig.h"
#include <stdint.h>


MORISA_NAMESPACE_BEGIN

enum KeyType : uint8_t
{
	kKeyTypeNone,
	kKeyTypeW,
	kKeyTypeS,
	kKeyTypeA,
	kKeyTypeD,
	kKeyTypeSpace,
	kKeyTypeLeftCtrl,
};

enum KeyAction : uint8_t
{
	kKeyActionRelease, 
	kKeyActionPress,   
	kKeyActionRepeat,	
};

struct KeyEvent
{ 
	static const uint32_t MAX_CONCURRENT_KEY = 4;
	uint32_t count;
	KeyType types[MAX_CONCURRENT_KEY];
	KeyAction actions[MAX_CONCURRENT_KEY];
};

struct MouseEvent
{
	float lastX;
	float lastY;
	float deltaX;
	float deltaY;
	const float epsilon;
	MouseEvent() :lastX(0.0), lastY(0.0), deltaX(0.0), deltaY(0.0), epsilon(1.0e-6)
	{
	}

	void Flush(float mouseX, float mouseY)
	{
		deltaX = mouseX - lastX;
		deltaY = mouseY - lastY;
		lastX = mouseX;
		lastY = mouseY;
	}

	const bool Active()const
	{
		return abs(deltaX) > epsilon || abs(deltaY) > epsilon;
	}
};

MORISA_NAMESPACE_END


#endif