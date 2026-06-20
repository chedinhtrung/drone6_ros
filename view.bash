ffplay \
  -fflags nobuffer+discardcorrupt \
  -flags low_delay \
  -framedrop \
  -probesize 32 \
  -analyzeduration 0 \
  -sync ext \
  -vf "setpts=0,vflip,hflip" \
  -f h264 \
  "udp://@:50001?fifo_size=10000&overrun_nonfatal=1"
