#include <iostream>
#include <cstring>
#include <csignal>
#include "ImagePipeline.h"
#include "V4L2Device.h"
#include "fcntl.h"
#include "PhysicalCamera.h"
#include "VirtualCamera.h"
#include <chrono>

volatile sig_atomic_t keepRunning = 1;

void signalHandler(int signum)
{
    keepRunning = 0;
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

    v4l2_format fmt = pc.getFormat();
    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    vc.setFormat(fmt);

    if (!pc.initMemory())
    {
        return -1;
    }

    ImagePipeline ip;
    ip.readArgs(argc, argv);

    // count fps
    int frameCount = 0;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!pc.setFramerate(30))
    {
        return -1;
    }

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
        ip.processImage(frame, bufferSize, pc.getBufferMaxSize());
        vc.writeBuffer(frame, bufferSize);
        pc.returnBuffer();

        frameCount++; // We successfully processed 1 frame

        // count fps step
        auto currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> elapsedTime = currentTime - startTime;
        if (elapsedTime.count() >= 1000.0)
        {
            std::cout << "\r[FPS] " << frameCount << std::flush;
            frameCount = 0;
            startTime = currentTime;
        }
    }

    std::cout << std::endl
              << "[SUCCESS] System stopped safely.\n";
    return 0;
}