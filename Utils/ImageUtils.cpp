#include "ImageUtils.h"
#include "LogUtils.h"
#include "MacroUtils.h"
#include "MathUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <string>
#include <cmath>

MORISA_NAMESPACE_BEGIN

MImage::MImage(const char* path):
    _data(nullptr)
    , _width(0)
    , _height(0)
    , _channel(0)
    , _size(0)
{
    // path nullptr is depth image
    if (path == nullptr)
    {
        return;
    }
    std::string realPath = globalConfig.resourcesRootPath;
    realPath.append(path);
    stbi_set_flip_vertically_on_load(true);
    stbi_uc* data = stbi_load(realPath.c_str(), &_width, &_height, &_channel, STBI_rgb_alpha);
    if (data == nullptr)
    {
        MORISA_LOG("ReadImage %s ERROR\n", path);
    }
    stbi_set_flip_vertically_on_load(false);
    _data = data;
    _size = _width;
    _size *= _height;
    // Force RGBA as the size
    _size *= 4;
}

MImage::~MImage()
{
    FreeImageReadMemory();
}

uint32_t MImage::MipLevels()
{
    return ((uint32_t)std::floor(std::log2(Max(_width, _height)))) + 1u; 
}

void MImage::FreeImageReadMemory()
{
    if (_data != nullptr)
    {
        stbi_image_free(_data);
        _data = nullptr;
    }
}


MORISA_NAMESPACE_END

