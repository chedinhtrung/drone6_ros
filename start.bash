source ./install/setup.bash

VIDEO_SINK="udp://192.168.0.103:50001"

ros2 run fcbridge bridge & 
rpicam-vid -t 0   --low-latency   --inline   --width 1280 --height 720 --framerate 25   --codec h264 --profile baseline   --intra 25   --nopreview   -o "$VIDEO_SINK?pkt_size=1316"

