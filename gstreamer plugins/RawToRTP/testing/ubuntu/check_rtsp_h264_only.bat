gst-launch-1.0 -v rtspsrc location=rtsp://127.0.0.1:8554/av1withrawcombined latency=0 ! rtph264depay ! h264parse ! avdec_h264 ! videoconvert ! autovideosink
