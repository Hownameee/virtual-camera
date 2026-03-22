#pragma once

#include <fcntl.h>
#include "V4L2Device.h"

class VirtualCamera : public V4L2Device
{
private:
public:
    VirtualCamera() : V4L2Device(V4L2_BUF_TYPE_VIDEO_OUTPUT) {}

    bool openDevice(const char *, int flags = O_WRONLY) override;
    void writeBuffer(void *buffer, size_t size);
};
