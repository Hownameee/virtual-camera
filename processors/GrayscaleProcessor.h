#pragma once
#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class GrayscaleProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- GRAYSCALE LOGIC START ---
        cv::cvtColor(img, img, cv::COLOR_BGR2GRAY);
        // Convert back to 3-channel BGR so the MJPEG encoder doesn't complain
        cv::cvtColor(img, img, cv::COLOR_GRAY2BGR); 
        // --- GRAYSCALE LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};