#pragma once

#include "processors/IProcessor.h"
#include <vector>
#include <functional>
#include <unordered_map>
#include <iostream>
#include <string>
#include <cstdint>

#include "deps/stb_image.h"
#include "deps/stb_image_write.h"

#include "processors/MirrorProcessor.h"
#include "processors/SepiaProcessor.h"
#include "processors/GrayscaleProcessor.h"

class ImagePipeline
{
private:
    std::vector<IProcessor *> steps;

    uint32_t pixelFormat;

    unsigned char *decodedPixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

public:
    ImagePipeline(uint32_t fmt)
    {
        pixelFormat = fmt;
    }

    ~ImagePipeline()
    {
        for (IProcessor *step : steps)
        {
            delete step;
        }
        if (decodedPixels)
        {
            stbi_image_free(decodedPixels);
        }
    }

    void addProcessor(IProcessor *step);
    void readArgs(int argc, char *argv[]);

    void decoder(void *buffer, size_t bufferSize);
    std::vector<uint8_t> process(void *buffer, size_t bufferSize, size_t maxBufferSize);
    std::vector<uint8_t> encoder();
};