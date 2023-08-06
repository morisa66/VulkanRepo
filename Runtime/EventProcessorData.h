#ifndef __EVENT_PROCESSOR_DATA_H__
#define __EVENT_PROCESSOR_DATA_H__

#include "Camera.h"
#include "Utils/EventUtils.h"

MORISA_NAMESPACE_BEGIN

struct FrameTime
{
	float current;
	float last;
	float delta;
	FrameTime() : current(0.0f), last(0.0f), delta(0.0f)
	{
	}
};

struct EventProcessorData
{
	Camera* camera;
	FrameTime time;
	KeyEvent keyEvent;
	MouseEvent mouseEvent;
	EventProcessorData(Camera* camera):
		camera(camera)
		, keyEvent()
		, mouseEvent()
		, time()
	{
		Reset();
	}

	void Reset()
	{
		keyEvent.count = 0;
	}
};

MORISA_NAMESPACE_END

#endif