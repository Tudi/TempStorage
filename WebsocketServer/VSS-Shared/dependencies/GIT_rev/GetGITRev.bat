@echo off
for /f "delims=" %%i in ('git rev-parse HEAD') do set GIT_HASH=%%i
echo #define GIT_HASH "%GIT_HASH%" > git_info.cpp