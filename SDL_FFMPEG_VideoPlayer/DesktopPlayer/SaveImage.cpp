#include <atlimage.h>
#include "StdAfx.h"

unsigned char clip(int nr)
{
	if (nr < 0)
		return 0;
	if (nr > 255)
		return 255;
	return nr;
}

int YUV420ToRGB32(int Y, int U, int V)
{
	unsigned char ret[4];
	int C = Y - 16;
	int D = U - 128;
	int E = V - 128;
	ret[0] = clip((298 * C + 409 * E + 128) >> 8); // R
	ret[1] = clip((298 * C - 100 * D - 208 * E + 128) >> 8); //G
	ret[2] = clip((298 * C + 516 * D + 128) >> 8); //B
	ret[3] = 0;
	return *((int*)ret);
}

int SaveAsBMP(AVFrame* frame, int Width, int Height, int FrameNumber)
{
	unsigned char* Ychannel = (unsigned char*)frame->data[0];
	int YStride = frame->linesize[0];
	unsigned char* Uchannel = (unsigned char*)(frame->data[1]);
	int UStride = frame->linesize[1];
	unsigned char* Vchannel = (unsigned char*)(frame->data[2]);
	int VStride = frame->linesize[2];
	//find an available file name
	char MyFileName[16000];
	sprintf_s(MyFileName, sizeof(MyFileName), "%s_%04d_%04d_%04d.bmp", "frame", FrameNumber, Width, Height);

	int HalfW = (Width + 1) / 2;
	int HalfH = (Height + 1) / 2;
	//create a bitmap and populate pixels on it
	CImage Img;
	Img.Create(Width, Height, 32);
	for (int y = 0; y < Height; y += 1)
		for (int x = 0; x < Width; x += 1)
		{
			int Y = Ychannel[y * YStride + x];
			int U = Uchannel[y / 2 * UStride + x / 2];
			int V = Vchannel[y / 2 * VStride + x / 2];
			Img.SetPixel(x, y, YUV420ToRGB32(Y, U, V));
		}

	GUID FileType = GUID_NULL;
	HRESULT res = Img.Save((LPCTSTR)MyFileName, FileType);

	return 0;
}

FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;

}

int SaveAsJPEG(unsigned char* YUV, int width, int height, const char* FileName, int FrameNumber)
{
	unsigned char* Ychannel = YUV;
	int YStride = width;
	unsigned char* Uchannel = Ychannel + width * height;
	int UStride = width/2;
	unsigned char* Vchannel = Uchannel + (width / 2) * (height / 2);
	int VStride = width / 2;

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	uint8_t* outbuffer = NULL;
	unsigned long outlen = 0;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &outbuffer, &outlen);

	// jrow is a libjpeg row of samples array of 1 row pointer
	cinfo.image_width = width & -1;
	cinfo.image_height = height & -1;
	cinfo.input_components = 3;
	cinfo.in_color_space = JCS_YCbCr; //libJPEG expects YUV 3bytes, 24bit

	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, 250, TRUE);
	jpeg_start_compress(&cinfo, TRUE);

	unsigned char* tmprowbuf = (unsigned char*)malloc(width * 3);

	JSAMPROW row_pointer[1];
	row_pointer[0] = &tmprowbuf[0];
	while (cinfo.next_scanline < cinfo.image_height)
	{
		int Row = cinfo.next_scanline;
		int RowC = Row / 2;
		for (unsigned i = 0; i < cinfo.image_width; i++)
		{
			tmprowbuf[i * 3 + 0] = Ychannel[YStride * Row + i]; // Y (unique to this pixel)
			tmprowbuf[i * 3 + 1] = Uchannel[UStride * RowC + i / 2]; // U (shared between pixels)
			tmprowbuf[i * 3 + 2] = Vchannel[VStride * RowC + i / 2]; // V (shared between pixels)
		}
		jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	char MyFileName[800];
	sprintf_s(MyFileName, sizeof(MyFileName), "%s_%04d_%04d_%04d.jpeg", FileName, FrameNumber, width, height);
	FILE* f;
	errno_t err = fopen_s(&f, MyFileName, "wb");
	if (f)
	{
		fwrite(outbuffer, 1, outlen, f);
		fclose(f);
		f = NULL;
	}

	jpeg_destroy_compress(&cinfo);

	free(tmprowbuf);
	free(outbuffer);
	return 0;
}

int SaveAsJPEG(AVFrame* frame, const char* FileName, int FrameNumber)
{
	int width = frame->width;
	int height = frame->height;
	unsigned char* Ychannel = (unsigned char*)frame->data[0];
	int YStride = frame->linesize[0];
	unsigned char* Uchannel = (unsigned char*)(frame->data[1]);
	int UStride = frame->linesize[1];
	unsigned char* Vchannel = (unsigned char*)(frame->data[2]);
	int VStride = frame->linesize[2];
	return SaveAsJPEG(Ychannel, width, height, FileName, FrameNumber);
}