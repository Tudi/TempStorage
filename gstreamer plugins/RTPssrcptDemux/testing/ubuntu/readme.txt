cd /mnt/d/GitHub/project_vast_gst/gst-av1ts/RTPssrcptDemux/

terminal 1 - generate "random" klv data for the plugin on port 5001
	python3 ./send_klv_udp.py

terminal 2 - generate random video RTP stream on port 5000
	h264
		gst-launch-1.0 videotestsrc is-live=true pattern=smpte ! video/x-raw,width=160,height=120,framerate=30/1 ! x264enc tune=zerolatency speed-preset=ultrafast byte-stream=true key-int-max=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5000
	AV1	
		gst-launch-1.0 videotestsrc is-live=true ! video/x-raw,width=160,height=120,framerate=30/1 ! rav1enc quantizer=150 ! av1parse ! rtpav1pay ! udpsink host=127.0.0.1 port=5000
	VP8
		gst-launch-1.0 videotestsrc is-live=true pattern=smpte ! video/x-raw,width=160,height=120,framerate=30/1 ! vp8enc deadline=1 ! rtpvp8pay pt=96 ! udpsink host=127.0.0.1 port=5000
	H265
		gst-launch-1.0 videotestsrc is-live=true pattern=smpte ! video/x-raw,width=160,height=120,framerate=30/1 ! x265enc tune=zerolatency speed-preset=ultrafast key-int-max=30 ! h265parse ! rtph265pay config-interval=1 pt=96 ! udpsink host=127.0.0.1 port=5000
	

terminal 2 - wrap custom KLV into RTP + merge with video stream and send it out on port 5002
			gst-launch-1.0 -v udpsrc port=5000 caps="application/x-rtp" ! identity silent=false ! queue ! udpsink host=172.22.16.1 port=5002 async=false sync=false udpsrc port=5001 caps="application/octet-stream" ! identity silent=false ! klvtortp maxupm=30 klvallowed=1,2 ! queue ! udpsink host=172.22.16.1 port=5002 async=false sync=false
			gst-launch-1.0 -v udpsrc port=5000 caps="application/x-rtp" ! queue ! udpsink host=172.22.16.1 port=5002 async=false sync=false udpsrc port=5001 caps="application/octet-stream" ! klvtortp maxupm=30 klvallowed=1,2 ! queue ! udpsink host=172.22.16.1 port=5002 async=false sync=false
	gst-launch-1.0 -v udpsrc port=5000 caps="application/x-rtp" ! queue ! udpsink host=127.0.0.1 port=5002 async=false sync=false udpsrc port=5001 caps="application/octet-stream" ! klvtortp maxupm=30 klvallowed=1,2 ! queue ! udpsink host=127.0.0.1 port=5002 async=false sync=false
	
testing the RTP streams :

	launch vast_palyer
	quick connect 
		connect to rtp source : 127.0.0.1:5002


debugging :
	- check if the plugin got installed :
		! plugin must be all lowercase named !
		set GST_DEBUG=GST_PLUGIN_LOADING:7
		set GST_DEBUG_FILE=debug.log
		gst-inspect-1.0 --gst-plugin-path=%CD% --print-plugin-auto-install-info > plugin_info.txt
		set GST_PLUGIN_PATH=%CD%
		gst-inspect-1.0 rtpssrcptdemux


test h264 and dump KLV to file :
	test if packets arrive : 	
			gst-launch-1.0 -v udpsrc port=5002 ! fakesink dump=true
	need to know the SSRC : 
			gst-launch-1.0 -v udpsrc port=5002 caps="application/x-rtp,clock-rate=90000" ! rtpssrcdemux name=demux demux.src_2590916116 ! application/x-rtp,media=video,encoding-name=H264,payload=96 ! queue ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink demux.src_1390770380 ! application/x-rtp,media=application,encoding-name=octet-stream,payload=97 ! identity silent=false single-segment=true ! queue ! filesink location=klv_dump.rtp
	dumps video as KLV : 
			gst-launch-1.0 -v udpsrc port=5002 caps="application/x-rtp" ! tee name=t t. ! queue ! "application/x-rtp, payload=96" ! rtph264depay ! avdec_h264 ! autovideosink t. ! queue ! filesink location=klv_dump.rtp
	no guessing of the codec :
				gst-launch-1.0 -v udpsrc port=5002 caps="application/x-rtp, clock-rate=90000" ! rtpssrcptdemux name=demux demux.src_96 ! capsfilter caps="application/x-rtp, media=video, encoding-name=H264, payload=96" ! queue ! rtph264depay ! h264parse ! avdec_h264 ! autovideosink demux.src_97 ! capsfilter caps="application/x-rtp, media=application, encoding-name=octet-stream, payload=97" ! identity silent=false single-segment=true ! queue ! filesink location=klv_dump.rtp sync=false
	guessing the codec, video only decoder
			gst-launch-1.0 -v udpsrc port=5002 caps="application/x-rtp, clock-rate=90000" ! rtpssrcptdemux name=demux guessmedia=true demux.src_96 ! decodebin ! videoconvert ! autovideosink