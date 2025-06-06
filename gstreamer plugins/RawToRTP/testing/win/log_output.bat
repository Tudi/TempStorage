cls
cd ../../x64/debug
set GST_PLUGIN_PATH=%CD%
SET GST_DEBUG=rawtortp:6 
set GST_DEBUG_FILE=debug.log
gst-launch-1.0 udpsrc port=5001 caps="application/octet-stream" ! rawtortp ! filesink location=output.rtp
cd ../../testing/win