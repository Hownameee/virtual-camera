#pragma once
#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class CartoonProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- CARTOON LOGIC START ---
        cv::Mat gray, edges, color;
        
        // 1. Get the outlines
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        cv::medianBlur(gray, gray, 5);
        cv::adaptiveThreshold(gray, edges, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 9, 9);
        
        // 2. Smooth the colors heavily (like a painting)
        cv::bilateralFilter(img, color, 9, 250, 250);
        
        // 3. Combine the smoothed colors with the harsh black outlines
        cv::bitwise_and(color, color, img, edges);
        // --- CARTOON LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};