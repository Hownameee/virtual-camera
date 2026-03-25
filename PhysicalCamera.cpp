#include "PhysicalCamera.h"
#include <linux/videodev2.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <iostream>
#include <cstring>

bool PhysicalCamera::openDevice(const char *path, int flags)
{
    return V4L2Device::openDevice(path, flags);
}

PhysicalCamera::~PhysicalCamera()
{
    if (bufferStart)
    {
        for (uint8_t i = 0; i < bufferCount; i++)
        {
            if (bufferStart[i] && bufferStart[i] != MAP_FAILED)
            {
                munmap(bufferStart[i], bufferMax);
            }
        }
        delete[] bufferStart;
        bufferStart = nullptr;
    }
}

bool PhysicalCamera::initMemory()
{
    // request buffer from kernel
    v4l2_requestbuffers rb{};
    rb.count = 4;
    rb.type = formatType;
    rb.memory = memoryType;
    if (ioctl(fd, VIDIOC_REQBUFS, &rb) == -1)
    {
        std::cerr << "[ERROR] REQBUFS failed.\n";
        return false;
    }

    bufferCount = rb.count;
    bufferStart = new void *[bufferCount]{};

    for (uint8_t i = 0; i < bufferCount; i++)
    {
        // query buffer (position, length, ...)
        v4l2_buffer buf{};
        buf.type = formatType;
        buf.memory = memoryType;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QUERYBUF, &buf) == -1)
        {
            std::cerr << "[ERROR] QUERYBUF failed at index " << i << "\n";
            return false;
        }

        // mapping pointer of user space to kernel space
        bufferMax = buf.length;
        bufferStart[i] = mmap(NULL, bufferMax, PROT_READ | PROT_WRITE, MAP_SHARED, fd, buf.m.offset);
        if (bufferStart[i] == MAP_FAILED)
        {
            std::cerr << "[ERROR] mmap failed at index " << i << "\n";
            return false;
        }
    }

    return true;
}

bool PhysicalCamera::startStreaming()
{
    for (uint8_t i = 0; i < bufferCount; i++)
    {
        // give buffer to kernel space for camera to write
        v4l2_buffer buf{};
        buf.type = formatType;
        buf.memory = memoryType;
        buf.index = i;
        if (ioctl(fd, VIDIOC_QBUF, &buf) == -1)
        {
            std::cerr << "[ERROR] QBUF failed at index " << i << ": " << strerror(errno) << "\n";
            return false;
        }
    }
    // turn on camera
    int type = formatType;
    if (ioctl(fd, VIDIOC_STREAMON, &type) == -1)
    {
        std::cerr << "[ERROR] STREAMON failed: " << strerror(errno) << "\n";
        return false;
    }
    return true;
}

void *PhysicalCamera::getFrame()
{
    v4l2_buffer buf{};
    buf.type = formatType;
    buf.memory = memoryType;
    // get buffer from kernel space
    if (ioctl(fd, VIDIOC_DQBUF, &buf) == -1)
        return nullptr;

    if (buf.index >= bufferCount)
    {
        std::cerr << "[ERROR] Kernel returned invalid buffer index\n";
        return nullptr;
    }

    bufferIndex = buf.index;
    bufferSize = buf.bytesused;

    return bufferStart[bufferIndex];
}

void PhysicalCamera::returnBuffer()
{
    // return buffer to kernel space
    struct v4l2_buffer buf{};
    buf.type = formatType;
    buf.memory = memoryType;
    buf.index = bufferIndex;
    ioctl(fd, VIDIOC_QBUF, &buf);
    bufferIndex = -1;
}

size_t PhysicalCamera::getBufferSize()
{
    return bufferSize;
}

size_t PhysicalCamera::getBufferMaxSize()
{
    return bufferMax;
}

bool PhysicalCamera::setFramerate(int fps)
{
    v4l2_streamparm parm{};
    parm.type = formatType;

    if (ioctl(fd, VIDIOC_G_PARM, &parm) == -1) {
        std::cerr << "[ERROR] Failed to get stream parameters.\n";
        return false;
    }

    // If the camera hardware doesn't support custom framerates, just exit happily
    if (!(parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) {
        return true; 
    }

    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = fps;

    if (ioctl(fd, VIDIOC_S_PARM, &parm) == -1) {
        std::cerr << "[ERROR] Failed to set FPS.\n";
        return false;
    }

    return true;
}

ConfigCamera PhysicalCamera::getHighestConfig()
{
    ConfigCamera bestConfig;
    v4l2_fmtdesc fmtdesc{};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    // loop through config of camera, default index = 0
    // VIDIOC_ENUM_FMT -> get pix format: mjpg, yuyv, ...
    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
    {
        v4l2_frmsizeenum frmsize{};
        frmsize.pixel_format = fmtdesc.pixelformat;

        // get size of that format
        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
        {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                uint32_t currentArea = frmsize.discrete.width * frmsize.discrete.height;
                uint32_t bestArea = bestConfig.width * bestConfig.height;

                // resolution first, fps second
                if (bestArea > currentArea)
                {
                    frmsize.index++;
                    continue;
                }

                if (currentArea > bestArea)
                {
                    bestConfig.fps = 0;
                }

                v4l2_frmivalenum frmival{};
                frmival.pixel_format = fmtdesc.pixelformat;
                frmival.width = frmsize.discrete.width;
                frmival.height = frmsize.discrete.height;

                // get frame interval or fps belong to pix format and size
                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0)
                {
                    if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
                    {
                        double currentFps = frmival.discrete.denominator * 1.0 / frmival.discrete.numerator;
                        if (currentFps > bestConfig.fps)
                        {
                            bestConfig.width = frmsize.discrete.width;
                            bestConfig.height = frmsize.discrete.height;
                            bestConfig.fps = currentFps;
                            bestConfig.pixelFormat = fmtdesc.pixelformat;
                            bestConfig.formatName = reinterpret_cast<const char *>(fmtdesc.description);
                        }
                    }
                    frmival.index++; // next interval
                }
            }
            frmsize.index++; // next frame size support
        }
        fmtdesc.index++; // next config
    }
    return bestConfig;
}