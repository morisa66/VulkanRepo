#include "VKInclude.h"

#include <vector>

MORISA_NAMESPACE_BEGIN

class VKRenderPass;
struct Attachment;

class VKFramebuffer
{
public:
	VKFramebuffer(VKRenderPass* renderPass);
	~VKFramebuffer();
	inline void SetCurrentBufferIndex(uint32_t currentindex) { _currentIndex = currentindex; }
	inline VkFramebuffer GetCurrentBuffer() { return _framebuffers[_currentIndex]; }
private:
	std::vector<VkFramebuffer> _framebuffers;
	uint32_t _currentIndex;
};

MORISA_NAMESPACE_END

