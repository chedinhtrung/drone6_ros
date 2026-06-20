source ../install/setup.bash
ros2 run drone6_bridge bridge
rpicam-vid -t 0   --low-latency   --inline   --width 1280 --height 720 --framerate 25   --codec h264 --profile baseline   --intra 25   --nopreview   -o "udp://192.168.0.103:50001?pkt_size=1316"

