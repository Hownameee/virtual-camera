#pragma once

#include "IImageProcess.h"

class GrayscaleProcessor : public IImageProcess
{
public:
    void process(unsigned char *decodedPixels, int width, int height, int channels) override
    {
        // Safety check
        if (!decodedPixels || width <= 0 || height <= 0 || channels < 3) {
            return;
        }

        int totalBytes = width * height * channels;

        // Loop through the flat array, jumping forward by the number of channels
        for (int i = 0; i < totalBytes; i += channels)
        {
            unsigned char r = decodedPixels[i];
            unsigned char g = decodedPixels[i + 1];
            unsigned char b = decodedPixels[i + 2];

            // Calculate luminance using human-eye-weighted math
            // Since the weights (0.299 + 0.587 + 0.114) equal exactly 1.0, 
            // the result will naturally stay between 0 and 255. No std::min required!
            unsigned char gray = static_cast<unsigned char>((r * 0.299f) + (g * 0.587f) + (b * 0.114f));

            // Set R, G, and B to the exact same gray value
            decodedPixels[i]     = gray;
            decodedPixels[i + 1] = gray;
            decodedPixels[i + 2] = gray;
        }
    }
};