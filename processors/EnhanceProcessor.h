#pragma once

#include "../IImageProcess.h"
#include <opencv2/opencv.hpp>
#include <vector>

class EnhanceProcessor : public IImageProcess {
public:
    void process(void *buffer, size_t &bufferSize, size_t maxBufferSize) override {
        if (!buffer || bufferSize == 0) return;

        cv::Mat rawData(1, bufferSize, CV_8UC1, buffer);
        cv::Mat img = cv::imdecode(rawData, cv::IMREAD_COLOR);
        if (img.empty()) return;

        // --- ENHANCE LOGIC START ---
        
        // 1. Fix Lighting (CLAHE)
        cv::Mat lab;
        cv::cvtColor(img, lab, cv::COLOR_BGR2Lab); // Convert to Lab color space
        std::vector<cv::Mat> lab_planes(3);
        cv::split(lab, lab_planes);  // Extract the L (Lightness) channel
        
        cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE(2.0, cv::Size(8, 8));
        clahe->apply(lab_planes[0], lab_planes[0]); // Apply contrast equalization
        
        cv::merge(lab_planes, lab);
        cv::cvtColor(lab, img, cv::COLOR_Lab2BGR); // Convert back to BGR

        // 2. Fix Soft Focus (Unsharp Masking)
        cv::Mat blurred;
        cv::GaussianBlur(img, blurred, cv::Size(0, 0), 3);
        cv::addWeighted(img, 1.5, blurred, -0.5, 0, img); // Sharpen the image

        // --- ENHANCE LOGIC END ---

        std::vector<uchar> outBuffer;
        cv::imencode(".jpg", img, outBuffer, {cv::IMWRITE_JPEG_QUALITY, 85});
        if (outBuffer.size() <= maxBufferSize) {
            std::memcpy(buffer, outBuffer.data(), outBuffer.size());
            bufferSize = outBuffer.size();
        }
    }
};