#include <cstring>
#include <iostream>
#include "VirtualCamera.h"

bool VirtualCamera::openDevice(const char *path, int flags)
{
    return V4L2Device::openDevice(path, flags);
}

void VirtualCamera::writeBuffer(void *buffer, size_t size)
{
    ssize_t bytes_written = write(fd, buffer, size);

    if (bytes_written < 0)
    {
        std::cerr << "[ERROR] Write to Virtual Camera failed: " << strerror(errno) << std::endl;
    }
    else if ((size_t)bytes_written < size)
    {
        std::cerr << "[WARNING] Not all data was written to Virtual Camera." << std::endl;
    }
}