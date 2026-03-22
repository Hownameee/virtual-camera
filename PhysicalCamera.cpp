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
    bufferStart = new void *[bufferCount];

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

    // get current fps
    if (ioctl(fd, VIDIOC_G_PARM, &parm) == -1)
    {
        std::cerr << "[ERROR] Failed to get current FPS configuration: " << strerror(errno) << "\n";
        return false;
    }

    if (!(parm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME))
    {
        std::cerr << "[ERROR] Camera driver does not support changing the framerate.\n";
        return false;
    }

    // Print current FPS
    int current_num = parm.parm.capture.timeperframe.numerator;
    int current_denom = parm.parm.capture.timeperframe.denominator;
    if (current_num != 0)
    {
        std::cout << "[INFO] Current Camera FPS is: "
                  << (current_denom / current_num) << "\n";
    }

    // set FPS
    parm.parm.capture.timeperframe.numerator = 1;
    parm.parm.capture.timeperframe.denominator = fps;
    if (ioctl(fd, VIDIOC_S_PARM, &parm) == -1)
    {
        std::cerr << "[ERROR] ioctl failed while setting FPS: " << strerror(errno) << "\n";
        return false;
    }

    // verify fps
    int actual_num = parm.parm.capture.timeperframe.numerator;
    int actual_denom = parm.parm.capture.timeperframe.denominator;

    if (actual_num == 0)
    {
        std::cerr << "[ERROR] Driver returned an invalid numerator (0).\n";
        return false;
    }

    int actual_fps = actual_denom / actual_num;

    if (actual_fps != fps)
    {
        std::cout << "[WARNING] Camera does not exactly support " << fps
                  << " FPS. Hardware automatically adjusted it to: " << actual_fps << " FPS.\n";
        // Still returning true since the setting was successfully applied by hardware limits.
    }
    else
    {
        std::cout << "[INFO] Successfully set the Camera to " << fps << " FPS!\n";
    }

    return true;
}

ConfigCamera PhysicalCamera::getHighestConfig()
{
    ConfigCamera bestConfig;
    v4l2_fmtdesc fmtdesc{};
    fmtdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    while (ioctl(fd, VIDIOC_ENUM_FMT, &fmtdesc) == 0)
    {
        struct v4l2_frmsizeenum frmsize;
        memset(&frmsize, 0, sizeof(frmsize));
        frmsize.pixel_format = fmtdesc.pixelformat;

        while (ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) == 0)
        {
            if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
            {
                uint32_t currentWidth = frmsize.discrete.width;
                uint32_t currentHeight = frmsize.discrete.height;
                uint32_t currentArea = currentWidth * currentHeight;

                struct v4l2_frmivalenum frmival;
                memset(&frmival, 0, sizeof(frmival));
                frmival.pixel_format = fmtdesc.pixelformat;
                frmival.width = currentWidth;
                frmival.height = currentHeight;

                while (ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) == 0)
                {
                    if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
                    {
                        double currentFps = static_cast<double>(frmival.discrete.denominator) /
                                            static_cast<double>(frmival.discrete.numerator);
                        uint32_t bestArea = bestConfig.width * bestConfig.height;
                        if ((currentArea > bestArea) ||
                            (currentArea == bestArea && currentFps > bestConfig.fps))
                        {
                            bestConfig.width = currentWidth;
                            bestConfig.height = currentHeight;
                            bestConfig.fps = currentFps;
                            bestConfig.pixelFormat = fmtdesc.pixelformat;
                            bestConfig.formatName = reinterpret_cast<const char *>(fmtdesc.description);
                        }
                    }
                    frmival.index++;
                }
            }
            frmsize.index++;
        }
        fmtdesc.index++;
    }
    return bestConfig;
}