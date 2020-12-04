#pragma once

#include <stdio.h>

//dump audio file as wav
int WriteWavHeader(const char* FileName, int AudioFormat, int channels, int bitsPerSample, int SampleRate, int FrameCount);
int ResampleAndWriteFrame(AVFrame* frame_, AVCodecContext* pCodecCtx, FILE *f);
int WriteWavBuffer(const char *FileName, unsigned char* buf, int Size);