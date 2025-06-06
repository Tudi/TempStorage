@echo off

REM Get WSL2 IP address
for /f "tokens=2 delims= " %%i in ('wsl ip -4 addr show eth0 ^| findstr "inet"') do (
    for /f "tokens=1 delims=/" %%a in ("%%i") do set WSLIP=%%a
)
echo WSL2 IP Address is %WSLIP%

gst-launch-1.0 videotestsrc is-live=true ! video/x-raw,width=160,height=120,framerate=30/1 ! rav1enc quantizer=150 ! av1parse ! rtpav1pay ! udpsink host=%WSLIP% port=5000
