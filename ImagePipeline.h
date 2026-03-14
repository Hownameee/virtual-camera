#pragma once

#include "IImageProcess.h"
#include <vector>

class ImagePipeline
{
private:
    std::vector<IImageProcess *> steps;

public:
    ~ImagePipeline()
    {
        for (IImageProcess *step : steps)
        {
            delete step;
        }
    }

    void addProcessor(IImageProcess *step)
    {
        steps.push_back(step);
    }

    void processImage(void *image)
    {
        for (IImageProcess *step : steps)
        {
            step->process(image);
        }
    }
};