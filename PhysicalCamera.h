#include <fcntl.h>
#include "V4L2Device.h"
#include "CameraBuffer.h"

class PhysicalCamera : public V4L2Device
{
private:
    size_t bufferLength;
    void *bufferStart = nullptr;
    v4l2_memory memoryType;

public:
    PhysicalCamera() : V4L2Device(V4L2_BUF_TYPE_VIDEO_CAPTURE) {
        memoryType = V4L2_MEMORY_MMAP;
    }
    bool openDevice(const char *, int flags = O_RDWR) override;
    
    void* getFrame();
    size_t getFrameSize();

    bool initMemory();
    void startStreaming();
    void returnBuffer();

};
