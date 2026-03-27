#pragma once

#include "IProcessor.h"
#include <cstring>

static const char *custom_bitmap[] = {
    "##   ##   #####   ##   ##  ##   ##   ######   ##   ##  ######",
    "##   ##  ##   ##  ##   ##  ###  ##  ##    ##  ### ###  ##     ",
    "#######  ##   ##  ## # ##  ## # ##  ########  ## # ##  ###### ",
    "##   ##  ##   ##  ### ###  ##  ###  ##    ##  ##   ##  ##     ",
    "##   ##   #####   ##   ##  ##   ##  ##    ##  ##   ##  ######"
};

class WatermarkProcessor : public IProcessor
{
public:
    void process(unsigned char *decodedPixels, int width, int height, int channels) override
    {
        int bmHeight = sizeof(custom_bitmap) / sizeof(custom_bitmap[0]);
        int bmWidth = strlen(custom_bitmap[0]);

        int scale = 12;

        int scaledWidth = bmWidth * scale;
        int scaledHeight = bmHeight * scale;

        int padding = 20;
        int startX = width - scaledWidth - padding;
        int startY = height - scaledHeight - padding;

        if (startX < 0)
            startX = 0;
        if (startY < 0)
            startY = 0;

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

                        if (imgX >= width || imgY >= height)
                            continue;

                        int idx = (imgY * width + imgX) * channels;

                        for (int c = 0; c < channels; c++)
                        {
                            int val = decodedPixels[idx + c] + 120;
                            decodedPixels[idx + c] = (val > 255) ? 255 : val;
                        }
                    }
                }
            }
        }
    }
};