cd ../../x64/debug
set GST_PLUGIN_PATH=%CD%
SET GST_DEBUG=rawtortp:6 
set GST_DEBUG_FILE=debug.log
gst-inspect-1.0 rawtortp
cd ../../testing/win
