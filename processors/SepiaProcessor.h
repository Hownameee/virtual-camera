#pragma once

#include "IImageProcess.h"
#include <algorithm> // Needed for std::min

class SepiaProcessor : public IImageProcess
{
public:
    void process(unsigned char *decodedPixels, int width, int height, int channels) override
    {
        // Safety check: Make sure we have pixels and at least 3 channels (RGB)
        if (!decodedPixels || width <= 0 || height <= 0 || channels < 3) {
            return; 
        }

        int totalBytes = width * height * channels;

        // Loop through the flat array, jumping forward by the number of channels (usually 3)
        for (int i = 0; i < totalBytes; i += channels)
        {
            // Extract the original RGB values
            // (stb_image forces standard RGB order, not BGR)
            unsigned char originalR = decodedPixels[i];
            unsigned char originalG = decodedPixels[i + 1];
            unsigned char originalB = decodedPixels[i + 2];

            // Apply the standard Sepia tone math
            int tr = static_cast<int>((originalR * 0.393f) + (originalG * 0.769f) + (originalB * 0.189f));
            int tg = static_cast<int>((originalR * 0.349f) + (originalG * 0.686f) + (originalB * 0.168f));
            int tb = static_cast<int>((originalR * 0.272f) + (originalG * 0.534f) + (originalB * 0.131f));

            // Overwrite the original pixels. 
            // We MUST cap the values at 255, otherwise they wrap around and cause glitchy neon pixels!
            decodedPixels[i]     = static_cast<unsigned char>(std::min(255, tr)); // R
            decodedPixels[i + 1] = static_cast<unsigned char>(std::min(255, tg)); // G
            decodedPixels[i + 2] = static_cast<unsigned char>(std::min(255, tb)); // B
            
            // Note: If your image has an Alpha channel (channels == 4), 
            // the loop naturally skips it because i += channels jumps right over it!
        }
    }
};