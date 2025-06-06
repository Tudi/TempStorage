cd ../../x64/release
set GST_PLUGIN_PATH=%CD%
SET GST_DEBUG=rtpssrcptdemux:6 
set GST_DEBUG_FILE=debug.log
gst-inspect-1.0 rtpssrcptdemux
cd ../../testing/win
