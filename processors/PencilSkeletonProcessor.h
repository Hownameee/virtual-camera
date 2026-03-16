#pragma once

#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class PencilSketchProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- SKETCH LOGIC START ---
        
        cv::Mat gray, inverted, blurred, sketch;
        
        // 1. Convert to grayscale
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        
        // 2. Invert the grayscale image
        cv::bitwise_not(gray, inverted);
        
        // 3. Blur the inverted image massively
        cv::GaussianBlur(inverted, blurred, cv::Size(21, 21), 0);
        
        // 4. Blend the grayscale and blurred images using color dodge
        cv::divide(gray, 255 - blurred, sketch, 256.0);
        
        // 5. Convert back to BGR so it matches the MJPEG color space expectations
        cv::cvtColor(sketch, img, cv::COLOR_GRAY2BGR);

        // --- SKETCH LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};