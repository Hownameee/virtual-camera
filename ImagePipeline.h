#pragma once

#include "IImageProcess.h"
#include <vector>

#include "processors/EnhanceProcessor.h"
#include "processors/MirrorProcessor.h"
#include "processors/PencilSkeletonProcessor.h"
#include "processors/SepiaProcessor.h"
#include "processors/PixelateProcessor.h"
#include "processors/NeonEdgeProcessor.h"
#include "processors/CartoonProcessor.h"
#include "processors/GrayscaleProcessor.h"

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

    void processImage(void *buffer, size_t &bufferSize, size_t maxBufferSize)
    {
        for (IImageProcess *step : steps)
        {
            step->process(buffer, bufferSize, maxBufferSize);
        }
    }

    void readArgs(int argc, char *argv[])
    {
        for (int i = 1; i < argc; i++)
        {
            if (std::strcmp(argv[i], "-m") == 0 || std::strcmp(argv[i], "--mirror") == 0)
            {
                this->addProcessor(new MirrorProcessor());
                std::cout << "[INFO] Added Mirror Processor.\n";
            }
            else if (std::strcmp(argv[i], "-e") == 0 || std::strcmp(argv[i], "--enhance") == 0)
            {
                this->addProcessor(new EnhanceProcessor());
                std::cout << "[INFO] Added Enhance (Quality) Processor.\n";
            }
            else if (std::strcmp(argv[i], "-s") == 0 || std::strcmp(argv[i], "--sepia") == 0)
            {
                this->addProcessor(new SepiaProcessor());
                std::cout << "[INFO] Added Sepia Filter Processor.\n";
            }
            else if (std::strcmp(argv[i], "-p") == 0 || std::strcmp(argv[i], "--pencil") == 0 || std::strcmp(argv[i], "--sketch") == 0)
            {
                this->addProcessor(new PencilSketchProcessor());
                std::cout << "[INFO] Added Pencil Sketch Processor.\n";
            }
            else if (std::strcmp(argv[i], "--pixel") == 0)
            {
                this->addProcessor(new PixelateProcessor());
                std::cout << "[INFO] Added Pixelate Processor.\n";
            }
            else if (std::strcmp(argv[i], "--neon") == 0)
            {
                this->addProcessor(new NeonEdgeProcessor());
                std::cout << "[INFO] Added Neon Edge Processor.\n";
            }
            else if (std::strcmp(argv[i], "--cartoon") == 0)
            {
                this->addProcessor(new CartoonProcessor());
                std::cout << "[INFO] Added Cartoon Processor.\n";
            }
            else if (std::strcmp(argv[i], "--gray") == 0)
            {
                this->addProcessor(new GrayscaleProcessor());
                std::cout << "[INFO] Added Grayscale Processor.\n";
            }
            else
            {
                std::cout << "[WARNING] Invalid or unknown flag: '" << argv[i] << "'\n";
            }
        }
    }
};