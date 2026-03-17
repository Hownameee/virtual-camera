#pragma once

#include "IImageProcess.h"
#include <vector>

#include "processors/MirrorProcessor.h"
#include "processors/SepiaProcessor.h"
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
        std::unordered_map<std::string, std::function<void()>> commandMap = {
            {"--mirror", [this]()
             { this->addProcessor(new MirrorProcessor()); std::cout << "[INFO] Added Mirror.\n"; }},
            {"--sepia", [this]()
             { this->addProcessor(new SepiaProcessor()); std::cout << "[INFO] Added Sepia.\n"; }},
            {"--gray", [this]()
             { this->addProcessor(new GrayscaleProcessor()); std::cout << "[INFO] Added Gray.\n"; }}};

        for (int i = 1; i < argc; i++)
        {
            if (auto it = commandMap.find(argv[i]); it != commandMap.end())
            {
                it->second();
            }
            else
            {
                std::cout << "[WARNING] Invalid or unknown flag: '" << argv[i] << "'\n";
            }
        }
    }
};