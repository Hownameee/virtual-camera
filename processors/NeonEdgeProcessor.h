#pragma once
#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class NeonEdgeProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- NEON EDGE LOGIC START ---
        cv::Mat edges;
        cv::cvtColor(img, edges, cv::COLOR_BGR2GRAY);
        
        // Detect edges (adjust 75 and 150 to catch more or fewer lines)
        cv::Canny(edges, edges, 75, 150); 
        
        // Create a black canvas
        img.setTo(cv::Scalar(0, 0, 0));
        
        // Draw the detected edges in Neon Green (BGR format: 0, 255, 0)
        img.setTo(cv::Scalar(0, 255, 0), edges);
        // --- NEON EDGE LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};