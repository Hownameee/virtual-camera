#include "V4L2Device.h"
#include <fcntl.h>
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <iostream>
#include <cstring>

bool V4L2Device::openDevice(const char *path, int type)
{
    if (isOpen())
    {
        close(fd);
    }

    fd = open(path, type);

    if (fd < 0)
    {
        return false;
    }
    return true;
}

v4l2_format V4L2Device::getFormat()
{
    if (!isOpen())
    {
        std::cerr << "[ERROR] Device is not open!" << std::endl;
        return {};
    }

    v4l2_format fmt{};
    fmt.type = formatType;
    if (ioctl(fd, VIDIOC_G_FMT, &fmt) == -1)
    {
        std::cerr << "[ERROR] VIDIOC_G_FMT failed: " << strerror(errno) << std::endl;
        return fmt;
    }
    return fmt;
}

void V4L2Device::setFormat(v4l2_format &format)
{
    if (!isOpen())
    {
        return;
    }
    format.type = formatType;
    if (ioctl(fd, VIDIOC_S_FMT, &format) == -1)
    {
        std::cerr << "[ERROR] VIDIOC_S_FMT failed: " << strerror(errno) << std::endl;
        return;
    }
}

bool V4L2Device::isOpen()
{
    return fd >= 0;
}