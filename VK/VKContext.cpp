#include "VKContext.h"

#include "VKCommandBuffer.h"
#include "VKBuffer.h"
#include "VKImage.h"
#include "VKDescriptor.h"
#include "VKSynchronization.h"
#include "VKRenderer.h"
#include "VKTools.h"
#include "VKUniform.h"
#include "VKMesh.h"
#include "VKPipeline.h"

#include "Runtime/Graphics/Material.h"

#include "Core/MVector.h"
#include "Core/MString.h"
#include <set>


MORISA_NAMESPACE_BEGIN

static VKContext* sContext = nullptr;

VKContext* Context()
{
    return sContext;
}

VkDevice Device()
{
    return sContext->Device();
}

VKContext::VKContext():
    _instance(VK_NULL_HANDLE)
    , _device(VK_NULL_HANDLE)
    , _physicalDevice(VK_NULL_HANDLE)
    , _graphicsQueueIndex(-1)
    , _computeQueueIndex(-1)
    , _transferQueueIndex(-1)
    , _presentQueueIndex(-1)
    , _queue(VK_NULL_HANDLE)
    , _window(nullptr)
    , _swapChain(nullptr)
    , _bufferManager(nullptr)
    , _commandBuffer(nullptr)
    , _descriptorManager(nullptr)
    , _uniformManager(nullptr)
    , _meshManager(nullptr)
    , _globalUniform(nullptr)
    , _pipelineCache(nullptr)
    , _synchronization(nullptr)
    , _renderer(nullptr)
{
    if (sContext != nullptr)
    {
        MORISA_LOG("VKContext duplicate create!\n");
        return;
    }

    sContext = this;

    CreateInstance();
    if (_instance == VK_NULL_HANDLE)
    {
        MORISA_LOG("CreateInstance ERROR\n");
        return;
    }
    
    PickUpGpu();
    if (_physicalDevice == VK_NULL_HANDLE)
    {
        MORISA_LOG("PickUpGpu ERROR\n");
        return;
    }

    _window = MORISA_NEW(VKWindow, globalConfig.defaultWindowWidth, globalConfig.defaultWindowHeight);

    SetQueueFamilies();
    if (_graphicsQueueIndex == -1)
    {
        MORISA_LOG("No suitable queue family index\n");
    }

    CreateDevice();
    if (_device == VK_NULL_HANDLE)
    {
        MORISA_LOG("CreateDevice ERROR\n");
    }

    VKCheckSafetyDelete::device = _device;
    GetQueue();

    _swapChain = MORISA_NEW(VKSwapChain);
    SetWindowSize(_swapChain->GetExtent());

    _commandBuffer = MORISA_NEW(VKCommandBuffer);
    _pipelineCache = MORISA_NEW(VKPipelineCache);
    _synchronization = MORISA_NEW(VKSynchronization);

    _bufferManager = MORISA_NEW(VKBufferManager);
    _imageManager = MORISA_NEW(VKImageManager);
    _descriptorManager = MORISA_NEW(VKDescriptorManager);
    _uniformManager = MORISA_NEW(VKUniformManager);
    _meshManager = MORISA_NEW(VKMeshManager);

    InitGlobalUniform();

    _renderer = MORISA_NEW(VKRenderer);
    
    _window->SetRenderer(_renderer);
}

VKContext::~VKContext()
{
    WaitIdle();

    MORISA_DELETE(_globalMaterial);

    MORISA_DELETE(_commandBuffer);
    MORISA_DELETE(_bufferManager);
    MORISA_DELETE(_imageManager);
    MORISA_DELETE(_descriptorManager);
    MORISA_DELETE(_uniformManager);
    MORISA_DELETE(_meshManager);
    MORISA_DELETE(_pipelineCache);
    MORISA_DELETE(_synchronization);

    MORISA_DELETE(_renderer);

    MORISA_DELETE(_swapChain);
    
    VK_VALID_DESTROY(_device, vkDestroyDevice, _device, nullptr);

    MORISA_DELETE(_window);

    VK_VALID_DESTROY(_instance, vkDestroyInstance, _instance, nullptr);
}

void VKContext::WaitIdle()
{
    VK_RESULT_CHECK(vkDeviceWaitIdle, _device);
}

void VKContext::WindowResize()
{
    WaitIdle();

    MORISA_DELETE(_swapChain);
    _swapChain = MORISA_NEW(VKSwapChain);
    SetWindowSize(_swapChain->GetExtent());

    _renderer->WindowResize();
}

void VKContext::CreateInstance()
{
    VkApplicationInfo applicationInfo{};
    applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    applicationInfo.pApplicationName = "GameRendering";
    applicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.pEngineName = "GameRendering";
    applicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    applicationInfo.apiVersion = VK_API_VERSION_1_3;

    uint32_t layerCount = 0;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    MVector<VkLayerProperties> layers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
    MVector<const char*> layerNames;

    for (const VkLayerProperties& layer : layers)
    {
        if (globalConfig.enableValidationLayer)
        {
            if (strcmp("VK_LAYER_KHRONOS_validation", layer.layerName) == 0)
            {
                layerNames.emplace_back("VK_LAYER_KHRONOS_validation");
            }
        }
        MORISA_LOG("InstanceLayerProperties %s\n", layer.layerName);
    }
    //layerNames.emplace_back("VK_LAYER_KHRONOS_synchronization2");
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr , &extensionCount, nullptr);
    MVector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
    MVector<const char*> extensionNames =
    {
        "VK_KHR_surface",
        "VK_KHR_win32_surface",
    };

    for (const VkExtensionProperties& extension : extensions)
    {
        MORISA_LOG("InstanceExtensionProperties %s\n", extension.extensionName);
    }

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &applicationInfo;
    instanceCreateInfo.flags = 0;
    instanceCreateInfo.pNext = nullptr;
    instanceCreateInfo.enabledLayerCount = layerNames.size();
    instanceCreateInfo.ppEnabledLayerNames = layerNames.data();
    instanceCreateInfo.enabledExtensionCount = extensionNames.size();
    instanceCreateInfo.ppEnabledExtensionNames = extensionNames.data();

    VK_RESULT_CHECK(vkCreateInstance, &instanceCreateInfo, nullptr, &_instance);
}

void VKContext::PickUpGpu()
{
    uint32_t gpuCount = 0;
    vkEnumeratePhysicalDevices(_instance, &gpuCount, nullptr);
    MVector<VkPhysicalDevice> gpus(gpuCount);
    vkEnumeratePhysicalDevices(_instance, &gpuCount, gpus.data());

    for (VkPhysicalDevice gpu : gpus)
    {
        vkGetPhysicalDeviceProperties(gpu, &_deviceProperties);
        vkGetPhysicalDeviceFeatures(gpu, &_deviceFeatures);
        if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            _physicalDevice = gpu;
            break;
        }
    }
}

void VKContext::SetQueueFamilies()
{
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, nullptr);
    MVector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(_physicalDevice, &queueFamilyCount, queueFamilies.data());
    std::set<uint32_t> graphicsQueueIndices;
    std::set<uint32_t> computeQueueIndices;
    std::set<uint32_t> transferQueueIndices;
    std::set<uint32_t> presentQueueIndices;
    for (uint32_t i = 0; i < queueFamilies.size(); ++i)
    {
        VkQueueFlags flags = queueFamilies[i].queueFlags;
        if (flags & VK_QUEUE_GRAPHICS_BIT)
        {
            graphicsQueueIndices.emplace(i);
        }
        if (flags & VK_QUEUE_COMPUTE_BIT)
        {
            computeQueueIndices.emplace(i);
        }
        if (flags & VK_QUEUE_TRANSFER_BIT)
        {
            transferQueueIndices.emplace(i);
        }
        VkBool32 supportPresent = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(_physicalDevice, i, _window->Surface(), &supportPresent);
        if (supportPresent)
        {
            presentQueueIndices.emplace(i);
        }
    }

    for (uint32_t index : graphicsQueueIndices)
    {
        // Choose same queue index
        if(computeQueueIndices.find(index) != computeQueueIndices.cend()
            && transferQueueIndices.find(index) != transferQueueIndices.cend()
            && presentQueueIndices.find(index) != presentQueueIndices.cend())
        {
            _graphicsQueueIndex = index;
            _computeQueueIndex = index;
            _transferQueueIndex = index;
            _presentQueueIndex = index;
        }
    }
    // TODO 
    // Process different queue index
}

void VKContext::CreateDevice()
{
    float queuePriority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.flags = 0;
    queueCreateInfo.queueFamilyIndex = _graphicsQueueIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    uint32_t layerCount = 0;
    vkEnumerateDeviceLayerProperties(_physicalDevice, &layerCount, nullptr);
    MVector<VkLayerProperties> layers(layerCount);
    vkEnumerateDeviceLayerProperties(_physicalDevice , &layerCount, layers.data());
    MVector<const char*> layerNames;
    for (const VkLayerProperties& layer : layers)
    {
        MORISA_LOG("DeviceLayerProperties %s\n", layer.layerName);
    }

    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, nullptr);
    MVector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(_physicalDevice, nullptr, &extensionCount, extensions.data());
    MVector<const char*> extensionNames =
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };
    for (const VkExtensionProperties& extension : extensions)
    {
        MORISA_LOG("DeviceExtensionProperties %s\n", extension.extensionName);
    }

    VkPhysicalDeviceFeatures _enabledDeviceFeatures{};

    _enabledDeviceFeatures.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.pNext = nullptr;
    deviceCreateInfo.flags = 0;
    // TODO 
    // Process different queue index
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
    deviceCreateInfo.enabledLayerCount = layerNames.size();
    deviceCreateInfo.ppEnabledLayerNames = layerNames.data();
    deviceCreateInfo.enabledExtensionCount = extensionNames.size();
    deviceCreateInfo.ppEnabledExtensionNames = extensionNames.data();
    deviceCreateInfo.pEnabledFeatures = &_enabledDeviceFeatures;

    VK_RESULT_CHECK(vkCreateDevice, _physicalDevice, &deviceCreateInfo, nullptr, &_device);
}

void VKContext::GetQueue()
{
    vkGetDeviceQueue(_device, _graphicsQueueIndex, 0, &_queue);
}

void VKContext::InitGlobalUniform()
{
    _globalMaterial = MORISA_NEW(MMaterial);
    _globalMaterial->Data(kMShaderStageVertex)->AddMat("View", glm::mat4(1.0f));
    _globalMaterial->Data(kMShaderStageVertex)->AddMat("Proj", glm::mat4(1.0f));
    // xyz position
    // w strength
    _globalMaterial->Data(kMShaderStageFragment)->AddVec("LightPosition", glm::vec4(2.0f, 2.0f, 2.0f, 2.5f));

    _globalUniform = UniformManager()->CreateUniform(_globalMaterial);
}


MORISA_NAMESPACE_END