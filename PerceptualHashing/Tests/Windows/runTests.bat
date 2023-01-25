@ECHO off
setlocal

REM test enrypt and burn
@SET CMD_OUT=
FOR /F %%I IN ('..\\..\\WaterMark\\x64\\Release\\WaterMark.exe 3 .\8300.png .\t.png WaterMarkSring EncryptorString') DO @SET "CMD_OUT=%%I"

if not "%CMD_OUT%" == "" echo "Error : Failed to burn watermark into image :'%CMD_OUT%'"


REM Test extract watermark and decrypt
@SET CMD_OUT=
FOR /F %%I IN ('..\\..\\WaterMark\\x64\\Release\\WaterMark.exe 2 .\t.png EncryptorString') DO @SET "CMD_OUT=%%I"

if not "%CMD_OUT%" == "WaterMarkSring"  echo "Error : Failed to burn/extract watermark into/from image :'%CMD_OUT%'" 


REM Test burn watermark and no encrypt
@SET CMD_OUT=
FOR /F %%I IN ('..\\..\\WaterMark\\x64\\Release\\WaterMark.exe 3 .\8300.png .\t.png WaterMarkSring \"\"') DO @SET "CMD_OUT=%%I"

if not "%CMD_OUT%" == "" echo "Error : Failed to burn watermark into image :'%CMD_OUT%'"


REM Test extract watermark and no decrypt
@SET CMD_OUT=
FOR /F %%I IN ('..\\..\\WaterMark\\x64\\Release\\WaterMark.exe 2 .\t.png \"\"') DO @SET "CMD_OUT=%%I"

if not "%CMD_OUT%" == "WaterMarkSring"  echo "Error : Failed to burn/extract watermark into/from image :'%CMD_OUT%'" 



echo "All tests are done"