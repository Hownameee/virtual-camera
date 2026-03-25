#pragma once

#include <unistd.h>
#include <cstdint>
#include <linux/videodev2.h>

class V4L2Device
{
protected:
    int fd = -1;
    unsigned int formatType;

public:
    V4L2Device(const unsigned int type)
    {
        formatType = type;
    }

    virtual ~V4L2Device()
    {
        if (fd >= 0)
        {
            close(fd);
        }
    }

    virtual bool openDevice(const char *, int);
    bool isOpen();

    v4l2_format getFormat();
    bool setFormat(v4l2_format &format);
};
