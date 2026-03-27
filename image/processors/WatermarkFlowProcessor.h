#pragma once

#include "IProcessor.h"
#include <cstring>
#include "bitmap.h"

class WatermarkFlowProcessor : public IProcessor
{
private:
    long long offsetX = 0;
    int speed = 40;

public:
    void process(unsigned char *decodedPixels, int width, int height, int channels) override
    {
        int bmHeight = sizeof(custom_bitmap) / sizeof(custom_bitmap[0]);
        int bmWidth = strlen(custom_bitmap[0]);

        int scale = 12;

        int scaledWidth = bmWidth * scale;
        int scaledHeight = bmHeight * scale;

        int paddingY = 20;
        int startY = height - scaledHeight - paddingY;
        if (startY < 0)
            startY = 0;

        offsetX += speed;

        if (offsetX > width)
            offsetX = -scaledWidth;

        int startX = offsetX;

        for (int y = 0; y < bmHeight; y++)
        {
            for (int x = 0; x < bmWidth; x++)
            {
                if (custom_bitmap[y][x] != '#')
                    continue;

                for (int dy = 0; dy < scale; dy++)
                {
                    for (int dx = 0; dx < scale; dx++)
                    {
                        int imgX = startX + x * scale + dx;
                        int imgY = startY + y * scale + dy;

                        if (imgX < 0 || imgX >= width || imgY >= height)
                            continue;

                        int idx = (imgY * width + imgX) * channels;

                        float alpha = 0.3f;

                        for (int c = 0; c < channels; c++)
                        {
                            decodedPixels[idx + c] =
                                decodedPixels[idx + c] * (1 - alpha) + 255 * alpha;
                        }
                    }
                }
            }
        }
    }
};