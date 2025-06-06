REM create a video source. In practice this could be a video file, a video camera, another stream
gst-launch-1.0 videotestsrc is-live=true ! video/x-raw,width=160,height=120,framerate=30/1 ! rav1enc quantizer=150 ! av1parse ! rtpav1pay ! udpsink host=127.0.0.1 port=5000
