#pragma once

#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <cstring>
#include <iostream>

class MirrorProcessor : public IImageProcess
{
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override;
};

void MirrorProcessor::process(void *buffer, size_t &bufferSize, size_t maxBufferSize)
{
    if (!buffer || bufferSize == 0)
    {
        return;
    }

    cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
    cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
    if (img.empty())
    {
        std::cerr << "[ERROR] OpenCV failed to decode the MJPEG buffer.\n";
        return;
    }

    cv::flip(img, img, 1);

    std::vector<uchar> outBuffer;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 85};
    cv::imencode(".jpg", img, outBuffer, params);

    if (outBuffer.size() <= maxBufferSize)
    {
        std::memcpy(buffer, outBuffer.data(), outBuffer.size());

        bufferSize = outBuffer.size();
    }
    else
    {
        std::cerr << "[ERROR] Processed image (" << outBuffer.size()
                  << " bytes) exceeds max buffer capacity (" << maxBufferSize << ").\n";
    }
}