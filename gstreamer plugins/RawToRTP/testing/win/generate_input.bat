gst-launch-1.0 -v audiotestsrc is-live=true wave=white-noise ! audio/x-raw,format=S16LE,channels=1,rate=8000 ! udpsink host=127.0.0.1 port=5000 sync=false
