#include <atlimage.h>

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

int SaveAsBMP(char* buff, int Width, int Height, int FrameNumber)
{
	//find an available file name
	char MyFileName[16000];
	sprintf_s(MyFileName, sizeof(MyFileName), "%s_%04d_%04d_%04d.bmp", "frame", FrameNumber, Width, Height);

	int HalfW = (Width + 1) / 2;
	int HalfH = (Height + 1) / 2;
	//create a bitmap and populate pixels on it
	CImage Img;
	Img.Create(Width, Height, 32);
	unsigned char* Ychannel = (unsigned char*)buff;
	unsigned char* Uchannel = (unsigned char*)(buff + Width * Height);
	unsigned char* Vchannel = (unsigned char*)(buff + HalfW * HalfH);
	for (int y = 0; y < Height; y += 1)
		for (int x = 0; x < Width; x += 1)
		{
			int Y = Ychannel[y * Width + x];
			int U = Uchannel[y / 2 * HalfW + x / 2];
			int V = Uchannel[y / 2 * HalfW + x / 2];
			Img.SetPixel(x, y, YUV420ToRGB32(Y, U, V));
		}

	GUID FileType = GUID_NULL;
	HRESULT res = Img.Save((LPCTSTR)MyFileName, FileType);

	return 0;
}

int SaveAsYUV(char* buff, int Width, int Height, int FrameNumber)
{
	//find an available file name
	char MyFileName[16000];
	sprintf_s(MyFileName, sizeof(MyFileName), "%s_%04d_%04d_%04d.Y4M", "frame", FrameNumber, Width, Height);

	FILE* f;
	errno_t open_res = fopen_s(&f, MyFileName, "wb");
	if (!f)
		return open_res;

	//write header
	sprintf_s(MyFileName, sizeof(MyFileName), "YUV4MPEG2 W%d H%d F24:1 Ip A0:0 C420mpeg2 XYSCSS=420MPEG2 FRAME", Width, Height);
	fwrite(MyFileName, 1, strlen(MyFileName), f);
	fwrite(buff, 1, Width * Height * 3 / 2, f);

	fclose(f);

	return 0;
}

int SaveImage(char *buff, int Width, int Height, int FrameNumber)
{
	return SaveAsBMP(buff, Width, Height, FrameNumber);
}