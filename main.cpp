#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include <iostream>
#include <cstring>
#include <csignal>
#include "processors/ImagePipeline.h"
#include "V4L2Device.h"
#include "fcntl.h"
#include "PhysicalCamera.h"
#include "VirtualCamera.h"
#include <chrono>

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int)
{
    keepRunning = 0;
}

void printConfig(ConfigCamera c)
{
    char formatStr[5] = {0};
    *reinterpret_cast<uint32_t*>(formatStr) = c.pixelFormat;

    std::cout << "\n========== Applied Camera Config ==========\n";
    std::cout << "[Resolution]   " << c.width << " x " << c.height << "\n";
    std::cout << "[Framerate]    " << c.fps << " FPS\n";
    std::cout << "[Pixel Format] " << formatStr << "\n";
    std::cout << "===========================================\n\n";
}

int main(int argc, char *argv[])
{
    // signal shutdown
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "[INFO] Starting Camera System..." << std::endl;

    // init camera
    PhysicalCamera pc;
    VirtualCamera vc;

    if (!pc.openDevice("/dev/video0"))
    {
        return -1;
    }
    if (!vc.openDevice("/dev/video2"))
    {
        return -1;
    }

    ConfigCamera cfg = pc.getHighestConfig();

    // config -> format
    v4l2_format fmt{};
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    fmt.fmt.pix.width = cfg.width;
    fmt.fmt.pix.height = cfg.height;
    fmt.fmt.pix.pixelformat = cfg.pixelFormat;
    fmt.fmt.pix.field = V4L2_FIELD_NONE;

    pc.setFormat(fmt);
    vc.setFormat(fmt);

    printConfig(cfg);

    if (!pc.setFramerate(cfg.fps)) {
        return -1;
    }

    if (!pc.initMemory())
    {
        return -1;
    }

    ImagePipeline ip;
    ip.readArgs(argc, argv);

    if (!pc.startStreaming())
    {
        return -1;
    }
    std::cout << "[INFO] Streaming started. Press Ctrl+C to stop.\n";

    while (keepRunning)
    {
        void *frame = pc.getFrame();
        if (!frame)
        {
            continue;
        }
        size_t bufferSize = pc.getBufferSize();
        std::vector<uint8_t> processedBuffer = ip.process(frame, bufferSize, pc.getBufferMaxSize());
        vc.writeBuffer(processedBuffer.data(), processedBuffer.size());
        pc.returnBuffer();
    }

    std::cout << std::endl
              << "[SUCCESS] System stopped safely.\n";
    return 0;
}