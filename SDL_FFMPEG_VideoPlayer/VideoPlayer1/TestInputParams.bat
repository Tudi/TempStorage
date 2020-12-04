"./x64/debug/VideoPlayer1.exe" 
"./x64/debug/VideoPlayer1.exe" in=TestSamples/small.mp4 
"./x64/debug/VideoPlayer1.exe" in=TestSamples/small.mp4 ShowFrameInfo=1
"./x64/debug/VideoPlayer1.exe" in=TestSamples/small.mp4 max_frames_to_decode=250
"./x64/debug/VideoPlayer1.exe" in=TestSamples/small.mp4 SaveImageStream=JPEG_out/Frame max_frames_to_decode=10
"./x64/debug/VideoPlayer1.exe" in=TestSamples/sample_02.mp4 SaveAudioStream=AudioChannel.wav max_frames_to_decode=750

"./VideoPlayer1/x64/Release/VideoPlayer1.exe" in=D:/test.mp4 SaveAudioStream=D:/test.wav max_frames_to_decode=5000