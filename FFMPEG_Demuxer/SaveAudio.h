#pragma once

#include <stdio.h>

//dump audio file as wav
int WriteWavHeader(FILE* f, int AudioFormat, int channels, int bitsPerSample, int SampleRate, int FrameCount);
int ResampleAndWriteFrame(AVFrame* frame_, AVCodecContext* pCodecCtx, FILE *f);