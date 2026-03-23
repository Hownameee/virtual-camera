# Virtual Camera

## Motivation (The Pain Point)

Many users find it disorienting when their standard webcam feed is not horizontally mirrored (like looking into a normal mirror). Most built-in camera drivers on Linux do not offer a native way to flip the image system-wide before it reaches the application.

## The Solution

This project creates a lightweight middleware process that sits between the physical hardware camera and end-user applications. The architecture works as follows:

1. **Read:** Automatically negotiates the highest supported resolution and framerate from the physical camera and captures the raw frame via the native V4L2 API.
2. **Process:** Decodes the frame, applies pixel-level manipulations (e.g., horizontal mirror, grayscale, or sepia), and re-encodes the image using ultra-fast, header-only `stb` C/C++ libraries.
3. **Write:** Pushes the processed frame into a virtual camera device at the OS kernel level.

Because the virtual camera is implemented at the kernel level, standard applications (like Zoom, Google Meet, OBS, or web browsers) detect it as a real, physical hardware camera. This ensures near-universal compatibility without requiring app-specific plugins.

## Dependencies

By replacing heavy frameworks like OpenCV with header-only alternatives, this project requires minimal system setup. You only need the following installed on your Ubuntu/Linux system:

* **`v4l2loopback-dkms`**: A kernel module used to create the virtual V4L2 video devices.
* **`build-essential` & `wget`**: Required for compiling the C++ code and automatically fetching the `stb` dependencies.

*(Note: The `stb_image.h` and `stb_image_write.h` headers are automatically downloaded into `image/deps/` when you run `make`).*

## Build & Run (CLI)

To set up the virtual camera and run the pipeline, execute the following commands in your terminal:

```bash
# 1. Clear any existing v4l2loopback instances
sudo rmmod v4l2loopback 

# 2. Create the virtual camera device (forces it to map to /dev/video2)
sudo modprobe v4l2loopback exclusive_caps=1 keep_format=1 video_nr=2 card_label="Virtual Camera"

# 3. Compile the C++ project (downloads deps and builds optimized binary)
make

# Note: For strict compiler warnings during development, you can use:
# make dev
```

## Usage

When you launch the executable, it will automatically print the optimized camera configuration it negotiated from your hardware.

You can stack multiple effects by passing command-line arguments. The pipeline will process them in the order they are initialized:

```bash
# Run normally (pass-through)
./main

# Run with a mirrored image
./main --mirror

# Stack multiple effects (e.g., Mirrored and Sepia)
./main --mirror --sepia
```
