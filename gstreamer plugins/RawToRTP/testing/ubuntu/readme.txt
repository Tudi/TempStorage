- start up a docker instance from the root directory
	docker run --rm -it -v %cd%:/workspace -p 8554:8554 -p 5000:5000/udp -p 5001:5001/udp gstreamer-dev-env
- get the docker instance id. Ex : 717ff8592a76
	docker ps
- create a new shell
	docker exec -it 717ff8592a76 bash
- make sure the new instance has everything that gstreamer needs
	apt update && apt install -y \
	  gir1.2-gst-rtsp-server-1.0 \
	  python3-gi \
	  python3-gi-cairo \
	  gstreamer1.0-tools \
	  gstreamer1.0-plugins-base \
	  gstreamer1.0-plugins-good \
	  gstreamer1.0-plugins-bad \
	  gstreamer1.0-plugins-ugly
- clean since this is a shared environment
	make clean
- build
	make
- install
	make install
- check if it got installed properly ( first run will take time since it's building a cache )
	export GST_DEBUG=rawtortp:6
	export GST_DEBUG_FILE=debug.log
	gst-inspect-1.0 rawtortp
- start sending raw data over UDP port 5001
	python3 ./testing/ubuntu/send_raw_udp.py
- open a new shell
- check that udp source is indeed visible
	apt update && apt install netcat-openbsd -y
	nc -ul 5001
- test if udp data is properly handled
	gst-launch-1.0 -v udpsrc port=5001 caps="application/octet-stream" ! identity silent=false ! rawtortp ! filesink location=output.rtp
- check the generated rtp packets are fine
	python3 ./testing/ubuntu/check_output_rtp.py ./output.rtp
- I could not make AV1 work on Ubuntu. My docker shares port 5000 with windows. I'm sending the AV1 stream from windows
	./testing/ubuntu/send_av1_udp.bat
- check av1 stream is visible inside docker container
	nc -ul 5000
- start the RTSP server
	python3 ./testing/ubuntu/rtsp_server.py
		or if you need to debug
	GST_DEBUG=rtsp*:6 python3 ./testing/ubuntu/rtsp_server.py 2>&1 | tee rtsp_debug.log
- check inside the container if the RTSP server is producing an output
	gst-launch-1.0 rtspsrc location=rtsp://127.0.0.1:8554/av1withrawcombined ! fakesink
- I can't replay video inside docker. I use windows to replay the RTP. Should see some small video content with colored lines
	recv_gstreamer.bat
	
	
	