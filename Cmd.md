# Run cmd

```bash
 sudo rmmod v4l2loopback 
 sudo modprobe v4l2loopback exclusive_caps=1 keep_format=1 video_nr=2 card_label="Virtual Camera"
 g++ *.cpp -o virtual_camera
 ./virtual_camera
```
