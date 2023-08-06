#ifndef __VK_WINDOW_H__
#define __VK_WINDOW_H__

#include "VKInclude.h"
#include <stdint.h>

struct GLFWwindow;

MORISA_NAMESPACE_BEGIN

class VKContext;
class VKRenderer;
struct EventProcessorData;

class VKWindow
{
public:
	VKWindow(uint32_t width, uint32_t height);
	~VKWindow();
	void RenderLoop();
	void SetRenderer(VKRenderer* renderer);
	inline VkSurfaceKHR Surface() { return _surface; }
	inline const VkExtent2D& Size() { return _size; }
	inline void SetSize(const VkExtent2D& size) { _size = size; }
	void RecreateWindow();
	void SetEventProcessorData(EventProcessorData* eventProcessorData);
	void PrepareEventProcessorData();
private:
	GLFWwindow* _window;
	VKRenderer* _renderer;
	VkInstance _instance;
	VkSurfaceKHR _surface;
	VkExtent2D _size;
	EventProcessorData* _eventProcessorData;
};

MORISA_NAMESPACE_END

#endif
