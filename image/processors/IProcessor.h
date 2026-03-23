#pragma once

class IProcessor
{
public:
    virtual ~IProcessor() = default;

    virtual void process(unsigned char *decodedPixels, int width, int height, int channels) = 0;
};