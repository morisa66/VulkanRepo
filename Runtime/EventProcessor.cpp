#include "EventProcessor.h"

MORISA_NAMESPACE_BEGIN

EventProcessor::EventProcessor()
{
}

EventProcessor::~EventProcessor()
{
}

void EventProcessor::Process(EventProcessorData* data)
{
	const KeyEvent& keyEvent = data->keyEvent;
	const MouseEvent& mouseEvent = data->mouseEvent;
	const FrameTime& time = data->time;
	Camera* camera = data->camera;
	if (keyEvent.count > 0)
	{
		for (int i = 0; i < keyEvent.count; ++i)
		{
			if (keyEvent.types[i] == kKeyTypeW)
			{
				camera->Move(kCameraDirectionForward, time.delta);
			}
			if (keyEvent.types[i] == kKeyTypeS)
			{
				camera->Move(kCameraDirectionBackward, time.delta);
			}
			if (keyEvent.types[i] == kKeyTypeA)
			{
				camera->Move(kCameraDirectionLeft, time.delta);
			}
			if (keyEvent.types[i] == kKeyTypeD)
			{
				camera->Move(kCameraDirectionRight, time.delta);
			}
			if (keyEvent.types[i] == kKeyTypeSpace)
			{
				camera->Move(kCameraDirectionUp, time.delta);
			}
			if (keyEvent.types[i] == kKeyTypeLeftCtrl)
			{
				camera->Move(kCameraDirectionDown, time.delta);
			}
		}
	}

	if (mouseEvent.Active())
	{
		camera->Rotate(mouseEvent.deltaX, -mouseEvent.deltaY);
	}

	data->Reset();
}


MORISA_NAMESPACE_END