#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "VKContext.h"
#include "VKWindow.h"
#include "VKRenderer.h"
#include "Runtime/EventProcessorData.h"

#include <unordered_map>

MORISA_NAMESPACE_BEGIN

static void FramebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	VKWindow* vkWindow = reinterpret_cast<VKWindow*>(glfwGetWindowUserPointer(window));
	vkWindow->RecreateWindow();
}

VKWindow::VKWindow( uint32_t width, uint32_t height):
	_window(nullptr)
	,_renderer(nullptr)
	,_surface(VK_NULL_HANDLE)
	, _instance(Context()->Instance())
	, _eventProcessorData(nullptr)
{
	_size.width = width;
	_size.height = height;
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	_window = glfwCreateWindow(width, height, "GameRendering", nullptr, nullptr);
	glfwSetWindowUserPointer(_window, this);
#if HIDE_CURSOR
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
#endif
	glfwSetFramebufferSizeCallback(_window, FramebufferResizeCallback);

	VkWin32SurfaceCreateInfoKHR win32SurfaceCreateInfoKHR{};
	win32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	win32SurfaceCreateInfoKHR.pNext = nullptr;
	win32SurfaceCreateInfoKHR.flags = 0;
	win32SurfaceCreateInfoKHR.hwnd = glfwGetWin32Window(_window);
	win32SurfaceCreateInfoKHR.hinstance = GetModuleHandle(nullptr);

	VK_RESULT_CHECK(vkCreateWin32SurfaceKHR, _instance, &win32SurfaceCreateInfoKHR, nullptr, &_surface);
}

VKWindow::~VKWindow()
{
	VK_VALID_DESTROY(_surface, vkDestroySurfaceKHR, _instance, _surface, nullptr);

	if (_window != nullptr)
	{
		glfwDestroyWindow(_window);
	}
	glfwTerminate();
}

void VKWindow::RenderLoop()
{
	while (!glfwWindowShouldClose(_window))
	{
		glfwPollEvents();

		PrepareEventProcessorData();

		_renderer->Update();

		_renderer->Sync();

		_renderer->RenderFrame();

		_renderer->AfterRender();
	}
}

void VKWindow::SetRenderer(VKRenderer* renderer)
{
	_renderer = renderer;
}

void VKWindow::RecreateWindow()
{
	int width = 0;
	int height = 0;
	glfwGetFramebufferSize(_window, &width, &height);
	if (width == 0 || height == 0) 
	{
		glfwGetFramebufferSize(_window, &width, &height);
		glfwWaitEvents();
		exit(0);
	}
	_size.width = width;
	_size.height = height;
	Context()->WindowResize();
}

void VKWindow::SetEventProcessorData(EventProcessorData* eventProcessorData)
{
	_eventProcessorData = eventProcessorData;
}

void VKWindow::PrepareEventProcessorData()
{
	typedef std::unordered_map<int, KeyAction> ActionMap;
	typedef std::unordered_map<int, KeyType> TypeMap;

	static const ActionMap actionMap
	{
		{GLFW_RELEASE, kKeyActionRelease},
		{GLFW_PRESS, kKeyActionPress},
		{GLFW_REPEAT, kKeyActionRepeat},
	};

	static const TypeMap typeMap
	{
		{GLFW_KEY_W, kKeyTypeW},
		{GLFW_KEY_S, kKeyTypeS},
		{GLFW_KEY_A, kKeyTypeA},
		{GLFW_KEY_D, kKeyTypeD},
		{GLFW_KEY_SPACE, kKeyTypeSpace},
		{GLFW_KEY_LEFT_CONTROL, kKeyTypeLeftCtrl},
	};

	static bool firstCall = true;

	if (glfwGetKey(_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(_window, GL_TRUE);
		return;
	}

	FrameTime& time = _eventProcessorData->time;
	time.current = (float)glfwGetTime();
	time.delta = time.current - time.last;
	time.last = time.current;

	KeyEvent& keyEvent = _eventProcessorData->keyEvent;
	for (TypeMap::const_iterator it = typeMap.begin(); it != typeMap.end(); ++it)
	{
		if (glfwGetKey(_window, it->first) == GLFW_PRESS)
		{
			keyEvent.actions[keyEvent.count] = kKeyActionPress;
			keyEvent.types[keyEvent.count] = it->second;
			keyEvent.count++; 
		}
	}

	double mouseX = 0.0;
	double mouseY = 0.0;
	glfwGetCursorPos(_window, &mouseX, &mouseY);
	if (firstCall)
	{
		_eventProcessorData->mouseEvent.lastX = (float)mouseX;
		_eventProcessorData->mouseEvent.lastY = (float)mouseY;
		firstCall = false;
	}
	_eventProcessorData->mouseEvent.Flush((float)mouseX, (float)mouseY);
}

MORISA_NAMESPACE_END


