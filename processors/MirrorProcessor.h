#pragma once

#include "IImageProcess.h"
#include <utility> // For std::swap

class MirrorProcessor : public IImageProcess
{
public:
    void process(unsigned char *decodedPixels, int width, int height, int channels) override
    {
        if (!decodedPixels || width <= 0 || height <= 0 || channels <= 0) {
            return; // Safety check
        }

        // Calculate how many bytes make up a single horizontal row
        int rowBytes = width * channels;

        // Loop through every single row from top to bottom
        for (int y = 0; y < height; ++y)
        {
            // Find the exact memory address where this row starts
            unsigned char* rowStart = decodedPixels + (y * rowBytes);

            // Loop halfway across the row to swap left and right pixels
            for (int x = 0; x < width / 2; ++x)
            {
                // Calculate the byte index of the left pixel and the right pixel
                int leftPixelIdx = x * channels;
                int rightPixelIdx = (width - 1 - x) * channels;

                // Swap the R, G, and B bytes individually
                for (int c = 0; c < channels; ++c)
                {
                    std::swap(rowStart[leftPixelIdx + c], rowStart[rightPixelIdx + c]);
                }
            }
        }
    }
};