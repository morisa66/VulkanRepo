#ifndef __EVENT_PROCESSOR_H__
#define __EVENT_PROCESSOR_H__

#include "EventProcessorData.h"

MORISA_NAMESPACE_BEGIN


class EventProcessor
{
public:
	EventProcessor();
	~EventProcessor();
	void Process(EventProcessorData* data);
private:
};


MORISA_NAMESPACE_END


#endif