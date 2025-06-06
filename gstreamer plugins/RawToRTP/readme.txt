Description :
	Plugin for GStreamer to wrap arbitrary raw binary data into RTP packets that could be restreamed by an RTSP server
	
Install :
	Windows :
		- read win_build.txt
	Ubuntu 24.04
		- make
		- make install

Test if installed. Should show pad info:
	gst-inspect-1.0 rawtortp
	
Testing on Windows:
	read ./testing/win/readme.txt
	
Testing for Ubuntu :
	read ./testing/ubuntu/readme.txt