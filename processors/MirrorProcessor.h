#pragma once

#include "../IImageProcess.h"

class MirrorProcessor : public IImageProcess
{
public:
    void process(void *buffer) override;
};

void MirrorProcessor::process(void *buffer)
{
}