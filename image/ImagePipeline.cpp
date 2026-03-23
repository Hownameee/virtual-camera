#include "ImagePipeline.h"
#include <linux/videodev2.h>

void ImagePipeline::addProcessor(IProcessor *step)
{
    steps.push_back(step);
}

std::vector<uint8_t> ImagePipeline::process(void *buffer, size_t bufferSize, size_t maxBufferSize)
{
    if (steps.empty())
    {
        auto *rawPtr = static_cast<uint8_t *>(buffer);
        return std::vector<uint8_t>(rawPtr, rawPtr + bufferSize);
    }

    decoder(buffer, bufferSize);

    if (decodedPixels)
    {
        for (IProcessor *step : steps)
        {
            step->process(decodedPixels, width, height, channels);
        }
    }

    std::vector<uint8_t> res = encoder();

    stbi_image_free(decodedPixels);
    decodedPixels = nullptr;

    return res;
}

void ImagePipeline::readArgs(int argc, char *argv[])
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

void ImagePipeline::decoder(void *buffer, size_t bufferSize)
{
    if (pixelFormat == V4L2_PIX_FMT_MJPEG)
    {
        decodedPixels = stbi_load_from_memory(static_cast<const stbi_uc *>(buffer), bufferSize, &width, &height, &channels, 3);
    }
}

void writeToVector(void *context, void *data, int size)
{
    auto *vec = static_cast<std::vector<uint8_t> *>(context);
    auto *byteData = static_cast<uint8_t *>(data);
    vec->insert(vec->end(), byteData, byteData + size);
}

std::vector<uint8_t> ImagePipeline::encoder()
{
    std::vector<uint8_t> res;
    if (pixelFormat == V4L2_PIX_FMT_MJPEG)
    {
        stbi_write_jpg_to_func(writeToVector, &res, width, height, channels, decodedPixels, 90);
    }
    return res;
}
