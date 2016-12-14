#!/bin/sh

gst-launch-1.0   -v -e v4l2src   device=/dev/v4l/by-id/usb-046d_HD_Pro_Webcam_C920_EA71325F-video-index0   !   video/x-h264,width=1280,height=720,framerate=30/1 ! h264parse !     rtph264pay !  udpsink host=192.168.230.71 port=1235
