#pragma once

#include <cstdint>

class IImageProcess
{
public:
    virtual ~IImageProcess() = default;

    virtual void process(void *image) = 0;
};