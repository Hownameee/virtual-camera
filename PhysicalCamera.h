#pragma once

#include <fcntl.h>
#include "V4L2Device.h"
#include "ConfigCamera.h"

class PhysicalCamera : public V4L2Device
{
private:
    v4l2_memory memoryType;
    uint8_t bufferCount;
    void **bufferStart = nullptr;
    int bufferIndex = -1;
    size_t bufferMax = 0;
    size_t bufferSize = 0;

public:
    PhysicalCamera() : V4L2Device(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    {
        memoryType = V4L2_MEMORY_MMAP;
    }
    ~PhysicalCamera() override;
    
    bool openDevice(const char *, int flags = O_RDWR) override;

    // config camera
    void *getFrame();
    size_t getBufferSize();
    size_t getBufferMaxSize();
    bool setFramerate(int fps);
    ConfigCamera getHighestConfig();

    // stream video
    bool initMemory();
    bool startStreaming();
    void returnBuffer();
};
