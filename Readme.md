# Linux Virtual Camera Pipeline

## Motivation (The Pain Point)

Many users find it disorienting when their standard webcam feed is not horizontally mirrored (like looking into a normal mirror). Most built-in camera drivers on Linux do not offer a native way to flip the image system-wide before it reaches the application.

## The Solution

This project creates a middleware process that sits between the physical hardware camera and end-user applications. The architecture works as follows:

1. **Read:** Captures the raw MJPEG frame from the physical camera via V4L2.
2. **Process:** Modifies the image using OpenCV (e.g., applying a horizontal mirror, grayscale, or cartoon effect).
3. **Write:** Pushes the processed frame into a virtual camera device at the OS kernel level.

Because the virtual camera is implemented at the kernel level, standard applications (like Zoom, Google Meet, OBS, or web browsers) detect it as a real, physical hardware camera. This ensures near-universal compatibility without requiring app-specific plugins.

## Dependencies

To build and run this project, you need the following installed on your Ubuntu/Linux system:

* **`v4l2loopback`**: A kernel module used to create the virtual V4L2 video devices.
* **`libopencv-dev`**: The OpenCV library for C++, used for real-time image decoding, manipulation, and encoding.

## Build & Run (CLI)

To set up the virtual camera and run the pipeline, execute the following commands in your terminal:

```bash
# 1. Clear any existing v4l2loopback instances
sudo rmmod v4l2loopback 

# 2. Create the virtual camera device (forces it to map to /dev/video2)
sudo modprobe v4l2loopback exclusive_caps=1 keep_format=1 video_nr=2 card_label="Virtual Camera"

# 3. Compile the C++ project using the Makefile
make

# 4. Run the executable
./main
```

## Known Issues & Future Improvements

* **Pipeline Performance Bottleneck:** Currently, the `ImagePipeline` architecture is not fully optimized for multiple chained processors. If you stack more than one filter (e.g., Mirror + Sepia), the system performs a heavy, redundant cycle for each processor:
  `Decode MJPEG -> Process Image -> Encode back to MJPEG`.
  This redundant compression/decompression severely impacts the FPS.
* **Upcoming Fix:** In a future update, the pipeline will be refactored to decode the MJPEG frame exactly once at the beginning, pass the raw `cv::Mat` through all active filters, and encode it back to MJPEG only once at the very end.
