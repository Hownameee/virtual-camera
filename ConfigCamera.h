#pragma once

#include <cstdint>
#include <string>

struct ConfigCamera
{
    uint32_t width = 0;
    uint32_t height = 0;
    double fps = 0.0;
    uint32_t pixelFormat = 0;
    std::string formatName = "Unknown";
};
