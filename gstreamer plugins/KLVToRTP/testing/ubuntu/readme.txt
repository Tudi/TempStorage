terminal 1 - generate "random" klv data for the plugin
	python3 ./testing/ubuntu/send_klv_udp.py

terminal 2 - generate random h264 video stream 
	gst-launch-1.0 videotestsrc is-live=true pattern=smpte ! video/x-raw,width=160,height=120,framerate=30/1 ! x264enc tune=zerolatency speed-preset=ultrafast byte-stream=true key-int-max=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5000

terminal 3 - create a RTSP server for the 2 RTP streams
		export GST_DEBUG=klvtortp:6
		export GST_DEBUG_FILE=debug.log
	python3 ./testing/ubuntu/rtsp_server_h264_klv.py

testing the RTP streams :
	launch vast_palyer
	quick connect 
		connect to rtsp source : 127.0.0.1:8554/vidwithklvcombined
