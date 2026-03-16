#pragma once
#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class PixelateProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- PIXELATE LOGIC START ---
        int blockSize = 15; // The larger the number, the chunkier the pixels
        cv::Mat smallImg;
        
        // Shrink the image
        cv::resize(img, smallImg, cv::Size(img.cols / blockSize, img.rows / blockSize), 0, 0, cv::INTER_LINEAR);
        
        // Enlarge it back using NEAREST neighbor to keep the blocky, jagged edges
        cv::resize(smallImg, img, img.size(), 0, 0, cv::INTER_NEAREST);
        // --- PIXELATE LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};