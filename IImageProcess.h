#pragma once

#include <cstdint>

class IImageProcess
{
public:
    virtual ~IImageProcess() = default;

    virtual void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) = 0;
};