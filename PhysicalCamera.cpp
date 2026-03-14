#include "PhysicalCamera.h"
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <iostream>

bool PhysicalCamera::openDevice(const char *path, int flags)
{
    return V4L2Device::openDevice(path, flags);
}

bool PhysicalCamera::initMemory()
{
    // request buffer from kernel
    v4l2_requestbuffers rb{};
    rb.count = 1;
    rb.type = formatType;
    rb.memory = memoryType;
    ioctl(fd, VIDIOC_REQBUFS, &rb);
    // query buffer (position, length, ...)
    v4l2_buffer buf{};
    buf.type = formatType;
    buf.memory = memoryType;
    buf.index = 0;
    ioctl(fd, VIDIOC_QUERYBUF, &buf);

    // mapping pointer of user space to kernel space
    bufferLength = buf.length;
    bufferStart = mmap(NULL, bufferLength, PROT_READ, MAP_SHARED, fd, buf.m.offset);

    return (bufferStart != MAP_FAILED);
}

void PhysicalCamera::startStreaming()
{
    v4l2_buffer buf{};
    buf.type = formatType;
    buf.memory = memoryType;
    buf.index = 0;
    // give buffer to kernel space for camera to write
    ioctl(fd, VIDIOC_QBUF, &buf);
    int type = buf.type;
    // turn on camera
    ioctl(fd, VIDIOC_STREAMON, &type);
}

void *PhysicalCamera::getFrame()
{
    v4l2_buffer buf{};
    buf.type = formatType;
    buf.memory = memoryType;
    // get buffer from kernel space
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
        return nullptr;
    return bufferStart;
}

void PhysicalCamera::returnBuffer()
{
    // return buffer to kernel space
    struct v4l2_buffer buf{};
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = 0;
    ioctl(fd, VIDIOC_QBUF, &buf);
}

size_t PhysicalCamera::getFrameSize()
{
    return bufferLength;
}