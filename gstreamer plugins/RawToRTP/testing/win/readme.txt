- open a cmd from this directory
- check that you actually have the plugin built properly
	check_its_registered.bat
- start generating udp content for the plugin to consume
	generate_input.bat
		or
	python send_raw_udp.py
- open a new cmd
- create a gstreamer instance that consumes the raw output and using the plugin wraps it into rtp packets
	log_output.bat
- check that the generated RTP packets are ok
	python check_output_rtp.py ../../x64/debug/output.rtp