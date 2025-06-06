cd /mnt/d/GitHub/project_vast_gst/gst-av1ts/RawToRTP/
sudo apt update && sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev libgstreamer-plugins-bad1.0-dev
sudo apt update && sudo apt install -y \
	  gir1.2-gst-rtsp-server-1.0 \
	  python3-gi \
	  python3-gi-cairo \
	  gstreamer1.0-tools \
	  gstreamer1.0-plugins-base \
	  gstreamer1.0-plugins-good \
	  gstreamer1.0-plugins-bad \
	  gstreamer1.0-plugins-ugly

make clean
make
make install
export GST_DEBUG=rawtortp:6
export GST_DEBUG_FILE=debug.log
gst-inspect-1.0 rawtortp
python3 ./testing/ubuntu/send_klv_udp.py
	optional tests here : 
		- open a new shell
		- check that udp source is indeed visible
			sudo apt update && sudo apt install netcat-openbsd -y
			nc -ul 5001
		- test if udp data is properly handled
			gst-launch-1.0 -v udpsrc port=5001 caps="application/octet-stream" ! identity silent=false ! rawtortp ! filesink location=output.rtp
		- check the generated rtp packets are fine
			python3 ./testing/ubuntu/check_output_rtp.py ./output.rtp
from windows : ./testing/ubuntu/send_av1_udp_wsl2.bat

GST_DEBUG=rtsp*:6 python3 ./testing/ubuntu/rtsp_server.py 2>&1 | tee rtsp_debug.log
	for debugging :
		GST_DEBUG=rtsp*:6 python3 ./testing/ubuntu/rtsp_server_h264_videoonly.py 2>&1 | tee rtsp_debug.log
		GST_DEBUG=rtsp*:6 python3 ./testing/ubuntu/rtsp_server_h264_raw.py 2>&1 | tee rtsp_debug.log
	
optional from windows check that RTSP is up and working : ./testing/ubuntu/recv_gstreamer.bat

from vast player :
	- open quick play
	- select rtsp
	- 127.0.0.1:8554/av1withrawcombined