Description:
	GStreamer plugin that will demux multiple RTP streams based on PT and SSRC
	It will create 1 pad for each incomming payloadtype
	It has optional parameter "guessmedia=true" that will try to wait for SDP info and set pad info. This can only happen for H264,H265,VP8,AV1 video streams. Only required if you do not know media type of hte payloadtype

Ubuntu build 
	make
	
Windows build :
	use visual studio 2022 and open RTPssrcptDemux.sln
	build Release x64
	
Ubuntu install
	make install
	
windows install :
	copy ./x64/Release/rtpssrcptdemux.dll
	to 
	C:\gstreamer\1.0\mingw_x86_64\lib\gstreamer-1.0\
	
usage example : 
	gst-launch-1.0 -v udpsrc port=5002 caps="application/x-rtp, clock-rate=90000" ! rtpssrcptdemux name=demux guessmedia=true demux.src_96 ! decodebin ! videoconvert ! autovideosink