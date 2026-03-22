#pragma once

#include <cstdint>

class IImageProcess
{
public:
    virtual ~IImageProcess() = default;

    virtual void process(unsigned char *decodedPixels, int width, int height, int channels) = 0;
};