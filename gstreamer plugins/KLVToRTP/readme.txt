Description :
	Plugin for GStreamer to wrap KLV data stream data into RTP packets that could be restreamed by an RTSP server
	Support KLV key filtering using 'klvallowed' plugin param
	Support max updates per second filtering using 'maxupm' plugin param
	
Example usage : 
	Example RTSP pipeline :
            '(udpsrc port=5000 caps="application/x-rtp, media=video, encoding-name=H264, payload=96, clock-rate=90000" ! '
            'rtph264depay ! h264parse config-interval=1 ! rtph264pay pt=96 name=pay0 ) '
            '( udpsrc port=5001 caps="application/octet-stream" '
            '! klvtortp maxupm=30 klvallowed=1,2 ! identity sync=false ! rtppassthroughpay pt=97 name=pay1 )'
		
Build & Install :
	Ubuntu 24.04
		- make
		- sudo make install
	Windows :
		- use Visual Studio 2022 to open : KLVToRTP.sln
		- should set custom plugin path to build output directory or copy to gstreamer plugins directory
	
Testing for Ubuntu :
	read ./testing/ubuntu/readme.txt