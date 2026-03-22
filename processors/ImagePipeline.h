#pragma once

#include "IImageProcess.h"
#include <vector>
#include <functional>
#include <unordered_map>

#include "stb_image.h"
#include "stb_image_write.h"

#include "MirrorProcessor.h"
#include "SepiaProcessor.h"
#include "GrayscaleProcessor.h"

void writeToVector(void *context, void *data, int size)
{
    auto *vec = static_cast<std::vector<uint8_t> *>(context);
    auto *byteData = static_cast<uint8_t *>(data);
    vec->insert(vec->end(), byteData, byteData + size);
}

class ImagePipeline
{
private:
    std::vector<IImageProcess *> steps;
    unsigned char *decodedPixels = nullptr;
    int width = 0;
    int height = 0;
    int channels = 0;

public:
    ~ImagePipeline()
    {
        for (IImageProcess *step : steps)
        {
            delete step;
        }
        if (decodedPixels)
        {
            stbi_image_free(decodedPixels);
        }
    }

    void addProcessor(IImageProcess *step)
    {
        steps.push_back(step);
    }

    std::vector<uint8_t> process(void *buffer, size_t bufferSize, size_t maxBufferSize)
    {
        std::vector<uint8_t> outBuffer;

        if (steps.empty())
        {
            auto *rawPtr = static_cast<uint8_t *>(buffer);
            return std::vector<uint8_t>(rawPtr, rawPtr + bufferSize);
        }

        preProcessImage(buffer, bufferSize);

        if (decodedPixels)
        {
            for (IImageProcess *step : steps)
            {
                step->process(decodedPixels, width, height, channels);
            }

            stbi_write_jpg_to_func(writeToVector, &outBuffer, width, height, channels, decodedPixels, 90);
        }

        return outBuffer;
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

    void preProcessImage(void *buffer, size_t bufferSize)
    {
        if (decodedPixels)
        {
            stbi_image_free(decodedPixels);
            decodedPixels = nullptr;
        }
        decodedPixels = stbi_load_from_memory(static_cast<const stbi_uc *>(buffer), bufferSize, &width, &height, &channels, 3);
    }
};