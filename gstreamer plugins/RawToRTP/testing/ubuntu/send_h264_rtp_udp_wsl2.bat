@echo off

REM Get WSL2 IP address
for /f "tokens=2 delims= " %%i in ('wsl ip -4 addr show eth0 ^| findstr "inet"') do (
    for /f "tokens=1 delims=/" %%a in ("%%i") do set WSLIP=%%a
)
echo WSL2 IP Address is %WSLIP%

gst-launch-1.0 videotestsrc is-live=true pattern=smpte ! video/x-raw,width=160,height=120,framerate=30/1 ! x264enc tune=zerolatency speed-preset=ultrafast byte-stream=true key-int-max=30 ! h264parse ! rtph264pay config-interval=1 pt=96 ! udpsink host=%WSLIP% port=5000
