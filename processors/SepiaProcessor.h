#pragma once

#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class SepiaProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- SEPIA LOGIC START ---
        
        // Define the Sepia transformation matrix
        cv::Mat sepiaKernel = (cv::Mat_<float>(3, 3) <<
            0.272, 0.534, 0.131, // Blue channel weights
            0.349, 0.686, 0.168, // Green channel weights
            0.393, 0.769, 0.189  // Red channel weights
        );
        
        // Apply the matrix transformation directly to the image
        cv::transform(img, img, sepiaKernel);

        // --- SEPIA LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};