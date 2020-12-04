"./VideoPlayer1/x64/release/VideoPlayer1.exe" in="D:\bT\Over\La Jetee (1970) CRITERION 720p BRRiP x264 AAC [Team Nanban]\La Jetee (1970) CRITERION 720p BRRiP x264 AAC [Team Nanban].mp4" ShowFrameInfo=1

"./VideoPlayer1/x64/release/VideoPlayer1.exe" in="D:\bT\Over\La Jetee (1970) CRITERION 720p BRRiP x264 AAC [Team Nanban]\La Jetee (1970) CRITERION 720p BRRiP x264 AAC [Team Nanban].mp4"

"./VideoPlayer1/x64/release/VideoPlayer1.exe" in=D:\bT\Over\Rick.and.Morty.S04E01.WEBRip.x264-TBS[ettv]\rick.and.morty.s04e01.webrip.x264-tbs[ettv].mkv

# Goals of the project

* Create a desktop player + library for audio video decoding through ffmpeg internal libraries (libav*). Get metadata, play / pause, seek, decode frames, pixel format conversion etc.
* First target Windows but do everything to make it work on Mac OSX later on by using cross platform libraries, cross platform C / C++ and CMake
* Clean separation between GUI player and library, the former using the latter
* GUI Player : one console launched to watch for keystroke to control playback, use something like SDL to display frames in another window, watch for mouseclicks on that window (and also same keystrokes than on console) for frame seeking (click in the horizontal middle of the screen => go to frame (framesCount-1)/2, vertical position doesn't matter)

# Milestones


M1. Basic decoding into .jpg image sequence via command lines. Do not use an ffmpeg command line, I want the frames to be saved in .jpg from the result of video decoding (and not just call ffmpeg to encode the file into JPEG !)
Let's limit ourselves to .MP4 / .MOV and H264 4:2:0
Decoding the file => core library
saving into JPEG => desktop program using the library
M2. Print in the console file metadata and metadata for each frame decoded
M3. Decode audio and write audio tracks into .wav files specified in command line (same than M1. do not use ffmpeg to write audio..). This should be enough to understand how to write a WAVE file : http://soundfile.sapp.org/doc/WaveFormat/
Let's limit ourselves to AAC / WAVE in input for now
M4. Watch for keystrokes in console for play/pause, show current frame metadata, simplify frame metadata showing for current frame decoded
M5. Create the SDL window to display the current image being decoded "in real time"
M6. Frame seeking in the SDL window 
M7. Add more codecs and containers, study/discuss which ones would be difficult to add and why => implement more pixel conversions



# General notes about collaboration and work habits

* Keep me up to date with what you're working on 
* Please divide your work in small commits otherwise I cannot follow your work. Each action is a commit. If you're pushing one giant commit with everything there is no way for me to know and understand what you are working on
* Do not hesitate to comment the source code to explain what you're doing or temporary workarounds, but useless comments that repeat variables names should be avoided
* Work on your own branch and tell me when I can test the code (and at which commit ID) I'll merge it onto master or dev myself = no need for a merge request
* If you copy pasted code from somewhere else say it in the git commit and comment the code with the URL source. One copy paste = 1 commit then you can modify the code and commit so we can see the differences you added / modified from original example 
* if there are big files to upload it would be best to use Git LFS (it's enabled by default on Gitlab but you need to install this yourself locally !). You need to add the file in .gitattributes for Git LFS to track the file and not regular git. Please make your own Git LFS test on a private repo to be sure it's working for you. If you're not using a Git GUI you might need to initialize it manually (git lfs install, sometimes git lfs pull). If you see the content of a file as a hash then it's probably a Git LFS pointer so it means Git LFS didn't work. Limit is 10 GB per git repo. HUGE files like image sequences and video should rather be stored on a Dropbox / Google Drive we share
* Provide build explanation for any external resources or if some part of the project needs to be built to test it
* The repo is not just the code but your work history in general
* Share with me any command line you used (I need to be able to replicate your tests and follow what you do !) or external stuff that is not usually shared in the repo.
* Keeps a journal / logs of what your doings (for example running a command line, save the output of the terminal), even if there are errors. Could be useful for documentation / troubleshooting later on. Especially if you're on a tough problem thinking or testing stuff for hours, keep up a journal about what you're doing so I can follow your work without asking you on Upwork / Slack what's going on 
* Use the Upwork app to track hours (unless we are not working through Upwork anymore)