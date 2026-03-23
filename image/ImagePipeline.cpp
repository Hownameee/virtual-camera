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
    else
    {
        auto *rawPtr = static_cast<uint8_t *>(buffer);
        return std::vector<uint8_t>(rawPtr, rawPtr + bufferSize);
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
    else if (pixelFormat == V4L2_PIX_FMT_YUYV)
    {
        channels = 3;
        int totalPixels = width * height;

        decodedPixels = static_cast<unsigned char *>(malloc(totalPixels * channels));

        auto *yuyv = static_cast<unsigned char *>(buffer);
        int rgbIndex = 0;

        for (int i = 0; i < totalPixels * 2; i += 4)
        {
            int y0 = yuyv[i];
            int u = yuyv[i + 1];
            int y1 = yuyv[i + 2];
            int v = yuyv[i + 3];

            int c = y0 - 16;
            int d = u - 128;
            int e = v - 128;
            int c2 = y1 - 16;

            decodedPixels[rgbIndex] = std::clamp((298 * c + 409 * e + 128) >> 8, 0, 255);
            decodedPixels[rgbIndex + 1] = std::clamp((298 * c - 100 * d - 208 * e + 128) >> 8, 0, 255);
            decodedPixels[rgbIndex + 2] = std::clamp((298 * c + 516 * d + 128) >> 8, 0, 255);

            decodedPixels[rgbIndex + 3] = std::clamp((298 * c2 + 409 * e + 128) >> 8, 0, 255);
            decodedPixels[rgbIndex + 4] = std::clamp((298 * c2 - 100 * d - 208 * e + 128) >> 8, 0, 255);
            decodedPixels[rgbIndex + 5] = std::clamp((298 * c2 + 516 * d + 128) >> 8, 0, 255);

            rgbIndex += 6;
        }
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
    else if (pixelFormat == V4L2_PIX_FMT_YUYV)
    {
        int totalPixels = width * height;

        res.resize(totalPixels * 2);

        int yuyvIndex = 0;

        for (int i = 0; i < totalPixels * 3; i += 6)
        {
            int r0 = decodedPixels[i];
            int g0 = decodedPixels[i + 1];
            int b0 = decodedPixels[i + 2];

            int r1 = decodedPixels[i + 3];
            int g1 = decodedPixels[i + 4];
            int b1 = decodedPixels[i + 5];

            int y0 = ((66 * r0 + 129 * g0 + 25 * b0 + 128) >> 8) + 16;
            int y1 = ((66 * r1 + 129 * g1 + 25 * b1 + 128) >> 8) + 16;

            int rAvg = (r0 + r1) / 2;
            int gAvg = (g0 + g1) / 2;
            int bAvg = (b0 + b1) / 2;

            int u = ((-38 * rAvg - 74 * gAvg + 112 * bAvg + 128) >> 8) + 128;
            int v = ((112 * rAvg - 94 * gAvg - 18 * bAvg + 128) >> 8) + 128;

            res[yuyvIndex++] = static_cast<uint8_t>(std::clamp(y0, 0, 255));
            res[yuyvIndex++] = static_cast<uint8_t>(std::clamp(u, 0, 255));
            res[yuyvIndex++] = static_cast<uint8_t>(std::clamp(y1, 0, 255));
            res[yuyvIndex++] = static_cast<uint8_t>(std::clamp(v, 0, 255));
        }
    }
    return res;
}
