#include <iostream>
#include <cstring>
#include <csignal>
#include "ImagePipeline.h"
#include "processors/MirrorProcessor.h"
#include "V4L2Device.h"
#include "fcntl.h"
#include "PhysicalCamera.h"
#include "VirtualCamera.h"

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
    PhysicalCamera *pc = new PhysicalCamera();
    VirtualCamera *vc = new VirtualCamera();

    if (!pc->openDevice("/dev/video0"))
    {
        return -1;
    }
    if (!vc->openDevice("/dev/video2"))
    {
        return -1;
    }

    v4l2_format fmt = pc->getFormat();
    vc->setFormat(fmt);

    if (!pc->initMemory())
    {
        return -1;
    }
    pc->startStreaming();

    ImagePipeline *ip = new ImagePipeline();
    for (int i = 1; i < argc; i++)
    {
        if (std::strcmp(argv[i], "-m") == 0 || std::strcmp(argv[i], "--mirror") == 0)
        {
            ip->addProcessor(new MirrorProcessor());
            std::cout << "[INFO] Added Mirror Processor.\n";
        }
        else
        {
            std::cout << "[WARNING] Invalid flag: '" << argv[i] << "'\n";
        }
    }

    std::cout << "[INFO] Streaming started. Press Ctrl+C to stop.\n";

    while (keepRunning)
    {
        void *frame = pc->getFrame();
        if (!frame)
        {
            continue;
        }
        // ip->processImage(frame);
        vc->writeBuffer(frame, pc->getFrameSize());
        pc->returnBuffer();
    }

    std::cout << "\n[INFO] Shutting down gracefully...\n";
    delete pc;
    delete vc;
    delete ip;
    std::cout << "[SUCCESS] System stopped safely.\n";
    return 0;
}