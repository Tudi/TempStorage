rem test different container formats
rem ffmpeg -i small.mp4 small.3gp
ffmpeg -i small.mp4 small.asf
ffmpeg -i small.mp4 small.avi
ffmpeg -i small.mp4 small.flv
ffmpeg -i small.mp4 small.webm
ffmpeg -i small.mp4 small.mov
ffmpeg -i small.mp4 small.mpegts
rem ffmpeg -i small.mp4 small.swf
ffmpeg -i small.mp4 small.ogv


rem create multiple encoding option
ffmpeg -i small.mp4 -c:v flashsv small_flashsv.avi
ffmpeg -i small.mp4 -c:v h263 -vf scale=352:288 small_h263.avi
ffmpeg -i small.mp4 -c:v hevc small_hevc.avi
ffmpeg -i small.mp4 -c:v huffyuv small_huffyuv.avi
ffmpeg -i small.mp4 -c:v jpeg2000 small_jpeg2000.avi
ffmpeg -i small.mp4 -c:v mpeg2video small_mpeg2video.avi
ffmpeg -i small.mp4 -c:v msmpeg4v3 small_msmpeg4v3.avi
ffmpeg -i small.mp4 -c:v qtrle small_qtrle.avi
ffmpeg -i small.mp4 -c:v theora small_theora.avi
ffmpeg -i small.mp4 -c:v vp8 small_vp8.avi
ffmpeg -i small.mp4 -c:v vp9 small_vp9.avi
ffmpeg -i small.mp4 -c:v wmv1 small_wmv1.avi

rem audio / video only tests
ffmpeg -i small.mp4 -c copy -an small_noaudio.avi
ffmpeg -i small.mp4 -c copy -vn small_novideo.avi

rem pixel format test
ffmpeg -i small.mp4 -pix_fmt yuv444p small_yuv444p.mp4