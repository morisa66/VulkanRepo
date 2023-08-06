#ifndef __VK_PIPELINE_STATE__
#define __VK_PIPELINE_STATE__

#include "VKInclude.h"

#include <vector>

MORISA_NAMESPACE_BEGIN

class VKUniform;
class VKPipelineCache;
class VKMesh;
class VKDescriptorSet;

class VKPipelineState
{
    friend VKPipelineCache;
public:
    VKPipelineState();
    ~VKPipelineState();
    inline VkPipelineLayout Layout() { return _layout; }
    void UpdateViewport(const VkExtent2D& windowSize);
    void AddAttachmentState(bool useAlphaBlend);
    void ConfigureVertexInput(VKMesh* mesh);
    void ConfigureLayout(VKUniform* localUniform);
    void ConfigureSamples(VkSampleCountFlagBits samples);
private:
    void ConfigureBaseState();
private:
    VkPipelineLayout _layout;

    VkPipelineVertexInputStateCreateInfo _vertexInputState;
    VkPipelineInputAssemblyStateCreateInfo _inputAssemblyState;
    VkPipelineTessellationStateCreateInfo _tessellationState;
    VkPipelineViewportStateCreateInfo _viewportState;
    VkPipelineRasterizationStateCreateInfo _rasterizationState;
    VkPipelineMultisampleStateCreateInfo _multisampleState;
    VkPipelineDepthStencilStateCreateInfo _depthStencilState;
    VkPipelineColorBlendStateCreateInfo _colorBlendState;
    VkPipelineDynamicStateCreateInfo _dynamicState;
    VkViewport _viewport;
    VkRect2D _scissor;
    std::vector<VkPipelineColorBlendAttachmentState> _attachmentStates;
    const std::vector<VkDynamicState> _dynamic{ VK_DYNAMIC_STATE_VIEWPORT , VK_DYNAMIC_STATE_SCISSOR };
    std::vector<VkVertexInputBindingDescription> bindingDescriptions;
};


MORISA_NAMESPACE_END

#endif
