#include <windows.h>
#include "StdAfx.h"
#include "FontLib.h"
#include <string>
#include <sys/stat.h>
#include "ShapeExtract.h"
#include "FontLib.h"

static int FontUIDCounter = 0;
std::list< FontImg*> CachedFonts;
void OCR_LoadFont(const char* Path, const char *AssignedString)
{
	FIBITMAP* dib = NULL;
	dib = LoadImage_(Path);
	if (dib == NULL)
	{
		printf("Could not load font : %s\n", Path);
		return;
	}
	int bpp = FreeImage_GetBPP(dib);
	if (bpp != 24)
		dib = FreeImage_ConvertTo24Bits(dib);
	bpp = FreeImage_GetBPP(dib);
	if (bpp != 24)
	{
		printf("!!!!Only support 24 bpp input. Upgrade software or convert input from %d to 24\n", bpp);
		return;
	}

	FontImg* i = (FontImg*)malloc(sizeof(FontImg));
	memset(i, 0, sizeof(FontImg));
	i->AssignedString = _strdup(AssignedString);
	i->Width = FreeImage_GetWidth(dib);
	i->Height = FreeImage_GetHeight(dib);
	i->pitch = FreeImage_GetPitch(dib);
	i->LoadedFromFile = 1;
	i->Scale = 1;
	i->UID = FontUIDCounter++;
	i->Pixels = (BYTE*)malloc(i->Height * i->pitch);
	memcpy(i->Pixels, FreeImage_GetBits(dib), i->Height * i->pitch);
	i->PixelsBlurred = NULL;

	CachedFonts.push_back(i);

	FreeImage_Unload(dib);
}

void OCR_LoadFontsFromDir(const char* Path)
{
	int skiptocharpos = (int)strlen(Path);
	std::string search_path = Path;
	search_path += "/*.*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do {
			// read all (real) files in current folder
			// , delete '!' read other 2 default folder . and ..
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
//				if (strstr(fd.cFileName, Path) != fd.cFileName)
//					continue;
				char FullPath[_MAX_PATH];
				sprintf_s(FullPath, sizeof(FullPath), "%s/%s", Path, fd.cFileName);
				//				printf("caching %s\n", FullPath);
				char c[_MAX_PATH];
				memset(c, 0, sizeof(c));
//				char* csrc = &fd.cFileName[skiptocharpos];
				char* csrc = &fd.cFileName[0];
				char* c2 = c;
				while (*csrc != ' ' && *csrc != 0 && *csrc != '.')
				{
					*c2 = *csrc;
					c2++;
					csrc++;
				}
				OCR_LoadFont(FullPath, c);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	printf("Loaded %d images for fonts\n", (int)CachedFonts.size());
}

void OCR_GenMultiScaleFonts()
{
	//let's get the size of a number, from there we will try to scale down / up to obtain multiple options
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		if ((*itr)->LoadedFromFile != 1)
			continue;

		FontImg* i = (*itr);

		int MaxSize = MAX(i->Width, i->Height);
		float AspectRatio = (float)i->Width/(float)i->Height;
		for (int DecreaseSizeByXPixels = 1; DecreaseSizeByXPixels < MaxSize - 5; DecreaseSizeByXPixels++)
		{
			//unrecognizable small font ? Could be anything
			int NewWidth;
			int NewHeight;
			if (i->Width > i->Height)
			{
				NewWidth = i->Width - DecreaseSizeByXPixels;
				NewHeight = i->Height - (int)((float)DecreaseSizeByXPixels / AspectRatio);
			}
			else
			{
				NewWidth = i->Width - (int)((float)DecreaseSizeByXPixels * AspectRatio);
				NewHeight = i->Height - DecreaseSizeByXPixels;
			}
			if (NewWidth <= 1)
				NewWidth = 1;
			if (NewHeight <= 5)
				break;
			int NewPitch = NewWidth * Bytespp;

			//scale the shape to the size of the current font image
			BYTE* ScaledShape = RescaleImgSubPixel(i->Pixels, i->Width, i->Height, i->pitch, NewWidth, NewHeight);
#ifdef _DEBUG
			//		SaveImagePNG(i->Pixels, i->Width, i->Height, i->pitch, "oldFont.png");
			char FileName[500];
			sprintf_s(FileName, sizeof(FileName), "FontsRescaledDebug/%s %d_%d.png", i->AssignedString, NewWidth, NewHeight);
			SaveImagePNG(ScaledShape, NewWidth, NewHeight, NewPitch, FileName);
#endif

			FontImg* i2 = (FontImg*)malloc(sizeof(FontImg));
			memset(i2, 0, sizeof(FontImg));
			i2->AssignedString = _strdup(i->AssignedString);
			i2->Width = NewWidth;
			i2->Height = NewHeight;
			i2->pitch = NewPitch;
			i2->LoadedFromFile = 0;
			i2->Scale = (float)(i->Width * i->Height) / (float(NewWidth * NewHeight));
			i2->UID = FontUIDCounter++;
			i2->Pixels = ScaledShape;

			CachedFonts.push_back(i2);
		}
	}
}

void OCR_GenBlurredFonts(int KernelSize = 4)
{
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		if ((*itr)->LoadedFromFile != 1)
			continue;
		FontImg* i = (*itr);
		i->PixelsBlurred = (BYTE*)malloc(i->Height * i->pitch);
		memcpy(i->PixelsBlurred, i->Pixels, i->Height * i->pitch);
		BlurrImageToGrayScale(i->PixelsBlurred, i->Width, i->Height, i->pitch, KernelSize);
		//BlurrImageToGrayScaleIfBlack(i->PixelsBlurred, i->Width, i->Height, i->pitch, KernelSize);
	}
}

void OCR_SaveFontVisualDebug()
{
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		FontImg* i = (*itr);
		char FileName[_MAX_PATH];
		static int FileNameIndex = 0;
		sprintf_s(FileName, sizeof(FileName), "./FontDebug/%s %d.png", i->AssignedString, FileNameIndex);
		FileNameIndex++;

		FIBITMAP* dib2 = FreeImage_Allocate(i->Width, i->Height, Bytespp * 8);
		int pitch2 = FreeImage_GetPitch(dib2);
		BYTE* Pixels2 = FreeImage_GetBits(dib2);

		if (pitch2 != i->pitch)
		{
			for (int y = 0; y < i->Width; y++)
				for (int x = 0; x < i->Height; x++)
				{
					Pixels2[y * pitch2 + x * Bytespp + 0] = i->Pixels[y * i->pitch + x * Bytespp + 0];
					Pixels2[y * pitch2 + x * Bytespp + 1] = i->Pixels[y * i->pitch + x * Bytespp + 1];
					Pixels2[y * pitch2 + x * Bytespp + 2] = i->Pixels[y * i->pitch + x * Bytespp + 2];
				}
		}
		else
			memcpy(Pixels2, i->Pixels, i->Height * pitch2);

		FreeImage_Save(FIF_PNG, dib2, FileName, 0);
	}
}

int OCR_GetNumbersAvgFontHeight()
{
	int HeightSum = 0;
	int HeightCount = 0;
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		FontImg* i = (*itr);
		if (i->AssignedString[1] != 0)
			continue;
		if (i->AssignedString[0] != '0'
			&& i->AssignedString[0] != '1'
			&& i->AssignedString[0] != '2'
			&& i->AssignedString[0] != '3'
			&& i->AssignedString[0] != '4'
			&& i->AssignedString[0] != '5'
			&& i->AssignedString[0] != '6'
			&& i->AssignedString[0] != '7'
			&& i->AssignedString[0] != '8'
			&& i->AssignedString[0] != '9'
			)
			continue;
		HeightSum += i->Height;
		HeightCount++;
	}
	if (HeightCount == 0)
		return 0;
	return HeightSum / HeightCount;
}

FontSimilarityScore* GetBestMatchedFont_FontDownScale(FontExtracted* f)
{
	FontSimilarityScore* ret = (FontSimilarityScore*)malloc(sizeof(FontSimilarityScore));
	memset(ret, 0, sizeof(FontSimilarityScore));
	ret->SAD = 0x0FFFFFFF;

	// dup the shape
	int pitchShape = f->Width * Bytespp;
	BYTE* fori = (BYTE*)malloc(f->Height * pitchShape);
	memcpy(fori, f->Pixels, f->Height * pitchShape);
	BlurrImageToGrayScale(fori, f->Width, f->Height, pitchShape, 4);
#if defined( _DEBUG ) && 0
	SaveImagePNG(fori, f->Width, f->Height, pitchShape, "curShape.png");
#endif
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		FontImg* i = (*itr);
		//scale the shape to the size of the current font image
		BYTE* ScaledShape = RescaleImg(i->Pixels, i->Width, i->Height, i->pitch, f->Width, f->Height);
//		BYTE* ScaledShape = RescaleImgSubPixel(i->Pixels, i->Width, i->Height, i->pitch, f->Width, f->Height);
		BlurrImageToGrayScale(ScaledShape, f->Width, f->Height, pitchShape, 4);
#if defined( _DEBUG ) && 0
		SaveImagePNG(i->Pixels, i->Width, i->Height, i->pitch, "oldFont.png");
		SaveImagePNG(ScaledShape, f->Width, f->Height, pitchShape, "curFont.png");
#endif
		//get the SAD of the 2 images
		__int64 SADNow = Img_SAD(fori, f->Width, f->Height, pitchShape, ScaledShape, pitchShape);
		//normalize SAD to original image size
		//if this is best possible match. note it
		if (SADNow < ret->SAD)
		{
			ret->SAD = SADNow;
			ret->fi = i;
		}
		free(ScaledShape);
	}
	free(fori);

	return ret;
}

FontSimilarityScore* GetBestMatchedFont_ShapeUpScale(FontExtracted* f)
{
	FontSimilarityScore* ret = (FontSimilarityScore*)malloc(sizeof(FontSimilarityScore));
	memset(ret, 0, sizeof(FontSimilarityScore));
	ret->SAD = 0xFFFFFFFFFFFFFF;
#ifdef _DEBUG
	SaveImagePNG(f->Pixels, f->Width, f->Height, f->Width * Bytespp, "OldShape.png");
#endif
	int pitchShape = f->Width * Bytespp;
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		FontImg* i = (*itr);
#if defined( ONLY_SIMILAR_ASPECT_RATIO ) || 0
		//try to not compare an "i" with an "m"
		float FontRatio = (float)i->Width / (float)i->Height;
		float ShapeRatio = (float)f->Width / (float)f->Height;
		if (FontRatio * 0.8 > ShapeRatio || FontRatio * 1.2 < ShapeRatio)
			continue;
#endif
		//scale the shape to the size of the current font image
		int pitchRescaled = i->Width * Bytespp;
		BYTE* ScaledShape = RescaleImg(f->Pixels, f->Width, f->Height, pitchShape, i->Width, i->Height);
//		BYTE* ScaledShape = RescaleImgSubPixel(f->Pixels, f->Width, f->Height, pitchShape, i->Width, i->Height);
#if defined(SCALE_SAD_TO_NEW_PIXEL_RATIO) || 0
		int ShapePixelCount = 0;
		for (int y = 0; y < i->Height; y++)
			for (int x = 0; x < i->Width; x++)
				if (ScaledShape[y * pitchRescaled + x * Bytespp])
					ShapePixelCount++;
		float PixelRatio = (float)ShapePixelCount / (float)f->PixelCount;
#endif
//		BlurrImageToGrayScale(ScaledShape, i->Width, i->Height, pitchRescaled, 3);
//		BlurrImageToGrayScaleIfBlack(ScaledShape, i->Width, i->Height, pitchRescaled, 2);
//		BlurrImageToGrayScale(ScaledShape, i->Width, i->Height, pitchRescaled, 4);
		//get the SAD of the 2 images
		__int64 SADNow = Img_SAD(i->Pixels, i->Width, i->Height, i->pitch, ScaledShape, pitchRescaled);
//		__int64 SADNow = Img_SAD(i->PixelsBlurred, i->Width, i->Height, i->pitch, ScaledShape, pitchRescaled);
//		__int64 SADNow = Img_SAD_SQ(i->Pixels, i->Width, i->Height, i->pitch, ScaledShape, pitchRescaled);

		//normalize SAD to original image size
#if defined(SCALE_SAD_TO_SINGLE_PIXEL) || 0
		__int64 PixelCount1 = i->Width * i->Height;
		__int64 NormalizedSAD = SADNow * 10000 / PixelCount1 / 3;
#endif

#if defined(SCALE_SAD_TO_SINGLE_PIXEL_FAVOR_ASPECT_RATIO) || 1
		float FontAspectRatio = (float)i->Width / (float)i->Height;
		float ShapeAspectRatio = (float)f->Width / (float)f->Height;
		float AspectRatioDiff = FontAspectRatio - ShapeAspectRatio;
		__int64 AspectRatioCoef = (__int64)(AspectRatioDiff * AspectRatioDiff * 128 * 10 * 10000);
		__int64 PixelCount1 = i->Width * i->Height;
		__int64 NormalizedSAD = SADNow * 10000 / PixelCount1 / 3 + AspectRatioCoef;
#endif

#if defined(USE_PIXEL_MATCH_COUNT) || 0
		int PixelMatchCount = 0;
		int PixelNoMatchCount = 0;
		int PixelExtraCount = 0;
		for (int ty = 0; ty < i->Height; ty++)
			for (int tx = 0; tx < i->Width; tx++)
			{
				if (i->Pixels[ty * i->pitch + tx * Bytespp] == 0)
				{
					if (ScaledShape[ty * pitchRescaled + tx * Bytespp] < 255)
						PixelMatchCount++;
					else
						PixelNoMatchCount++;
				}
				else if (ScaledShape[ty * pitchRescaled + tx * Bytespp] < 255)
					PixelExtraCount++;
			}
		__int64 NormalizedSAD = ( (PixelNoMatchCount + PixelExtraCount - PixelMatchCount) * 100000) / (i->Height*i->Height);
#endif

#if defined(USE_PIXEL_MATCH_COUNT) || 0
		ShapeMorphStatus sms;
		OCR_GetMorphCost(i->Pixels, i->Width, i->Height, i->pitch, ScaledShape, pitchRescaled, &sms);
		__int64 NormalizedSAD = sms.Distances * 1000 / sms.PixelsMigrated;
#endif

#if defined(REMOVE_SAD_EXTRA_SCALE) || 0
		__int64 NewPixelsAdded = i->Width * i->Height - f->Width * f->Height;
		__int64 NormalizedSAD = SADNow - NewPixelsAdded * 3 * 64;
#endif

#if defined(SCALE_SAD_TO_SINGLE_PIXEL_SQUARES) || 0
		__int64 PixelCount1 = i->Width * i->Height;
		__int64 NormalizedSAD = (__int64)sqrt(SADNow / PixelCount1 / 3);
#endif

#if defined(SCALE_SAD_TO_OLD_PIXEL_RATIO) || 0
		__int64 PixelCount2 = f->Width * f->Height;
		float SizeRatio = (float)PixelCount1 / (float)PixelCount2;
		__int64 NormalizedSAD = (__int64)((float)SADNow / SizeRatio);
#endif
		
#if defined(USE_SAD_FROM_FULL_RESIZED) || 0
		__int64 NormalizedSAD = SADNow;
#endif

#if defined(SCALE_SAD_TO_NEW_PIXEL_RATIO) || 0
		__int64 NormalizedSAD = (__int64)((float)SADNow / PixelRatio);
#endif

#if defined( _DEBUG ) && 0
		//merge the 2
		BYTE* timg = (BYTE*)malloc(i->Width * 2 * i->Height * Bytespp);
		int tpitch = i->Width * 2 * 3;
		for (int ty = 0; ty < i->Height; ty++)
			for (int tx = 0; tx < i->Width; tx++)
			{
				timg[ty * tpitch + tx * 3 + 0] = i->Pixels[ty * i->pitch + tx * 3 + 0];
				timg[ty * tpitch + tx * 3 + 1] = i->Pixels[ty * i->pitch + tx * 3 + 1];
				timg[ty * tpitch + tx * 3 + 2] = i->Pixels[ty * i->pitch + tx * 3 + 2];
				timg[ty * tpitch + (tx + i->Width) * 3 + 0] = ScaledShape[ty * pitchRescaled + tx * 3 + 0];
				timg[ty * tpitch + (tx + i->Width) * 3 + 1] = ScaledShape[ty * pitchRescaled + tx * 3 + 1];
				timg[ty * tpitch + (tx + i->Width) * 3 + 2] = ScaledShape[ty * pitchRescaled + tx * 3 + 2];
			}
		char tname[500];
//		sprintf_s(tname, sizeof(tname), "SADDebug/%09lld %s.png", NormalizedSAD, i->AssignedString);
		sprintf_s(tname, sizeof(tname), "SADDebug/%09lld %d %d %d %s.png", NormalizedSAD, PixelMatchCount, PixelNoMatchCount, PixelExtraCount,i->AssignedString);
		SaveImagePNG(timg, i->Width * 2, i->Height, tpitch, tname);
		free(timg);
		SaveImagePNG(i->Pixels, i->Width, i->Height, i->pitch, "oldFont.png");
		SaveImagePNG(ScaledShape, i->Width, i->Height, pitchRescaled, "curShape.png");
#endif
		//if this is best possible match. note it
		if (NormalizedSAD < ret->SAD)
		{
			ret->SAD = NormalizedSAD;
			ret->fi = i;
		}
		free(ScaledShape);
	}

	return ret;
}

FontSimilarityScore* GetBestMatchedFont_ShapeMatchFont(FontExtracted* f)
{
	FontSimilarityScore* ret = (FontSimilarityScore*)malloc(sizeof(FontSimilarityScore));
	memset(ret, 0, sizeof(FontSimilarityScore));
	ret->SAD = 0xFFFFFFFFFFFFFF;

	int pitchShape = f->Width * Bytespp;
	BYTE* Shape = (BYTE*)malloc(f->Height * pitchShape);
	memcpy(Shape, f->Pixels, f->Height * pitchShape);
//	BlurrImageToGrayScale(Shape, f->Width, f->Height, pitchShape, 2);

	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		FontImg* i = (*itr);

		//try to not compare an "i" with an "m"
		if (abs(i->Width - f->Width) > 2 || abs(i->Height - f->Height) > 2)
			continue;

		int MinWidth = MIN(i->Width, f->Width);
		int MinHeight = MIN(i->Height, f->Height);
		__int64 SADNow = Img_SAD_SQ(i->Pixels, MinWidth, MinHeight, i->pitch, Shape, pitchShape);

		//get the morph distance from 1 image to the next

		__int64 NormalizedSAD = SADNow / (MinWidth * MinHeight);
#if defined( _DEBUG ) && 0
		//merge the 2
		if (NormalizedSAD < ret->SAD)
		{
			int MaxWidth = MAX(i->Width, f->Width);
			int MaxHeight = MAX(i->Height, f->Height);

			BYTE* timg = (BYTE*)malloc((MaxWidth * 2 +1) * MaxHeight * Bytespp);
			int tpitch = (MaxWidth * 2 + 1) * Bytespp;
			//set a background
			for (int ty = 0; ty < MaxHeight; ty++)
				for (int tx = 0; tx < MaxWidth * 2 + 1; tx++)
					timg[ty * tpitch + tx * 3 + 1] = 255;
			//split the 2 images
			for (int ty = 0; ty < MaxHeight; ty++)
				timg[ty * tpitch + i->Width * 3 + 0] = 255;
			//put the images side by side
			for (int ty = 0; ty < MinHeight; ty++)
				for (int tx = 0; tx < MinWidth; tx++)
				{
					timg[ty * tpitch + tx * 3 + 0] = i->Pixels[ty * i->pitch + tx * 3 + 0];
					timg[ty * tpitch + tx * 3 + 1] = i->Pixels[ty * i->pitch + tx * 3 + 1];
					timg[ty * tpitch + tx * 3 + 2] = i->Pixels[ty * i->pitch + tx * 3 + 2];
					timg[ty * tpitch + (tx + 1 + i->Width) * 3 + 0] = Shape[ty * pitchShape + tx * 3 + 0];
					timg[ty * tpitch + (tx + 1 + i->Width) * 3 + 1] = Shape[ty * pitchShape + tx * 3 + 1];
					timg[ty * tpitch + (tx + 1 + i->Width) * 3 + 2] = Shape[ty * pitchShape + tx * 3 + 2];
				}
			char tname[500];
			sprintf_s(tname, sizeof(tname), "SADDebug/%d %09lld %s.png", f->ShapeID, NormalizedSAD, i->AssignedString);
			SaveImagePNG(timg, (MaxWidth * 2 + 1), MaxHeight, tpitch, tname);
			free(timg);
		}
#endif
		//if this is best possible match. note it
		if (NormalizedSAD < ret->SAD)
		{
			ret->SAD = NormalizedSAD;
			ret->fi = i;
		}
//		free(ScaledShape);
	}
	free(Shape);

	return ret;
}

FontSimilarityScore* GetBestMatchedFont_ShapeSourceMatchFont(FIBITMAP* dib, FontExtracted* f)
{
	FontSimilarityScore* ret = (FontSimilarityScore*)malloc(sizeof(FontSimilarityScore));
	memset(ret, 0, sizeof(FontSimilarityScore));
	ret->SAD = 0xFFFFFFFFFFFFFF;

#define ExtraBorderExtract 3
	int pitchShape = (f->Width + ExtraBorderExtract * 2) * Bytespp;
	BYTE* Shape = (BYTE*)malloc((f->Height + ExtraBorderExtract * 2) * pitchShape);
//	BYTE* Shape0 = &Shape[ExtraBorderExtract * pitchShape + ExtraBorderExtract * Bytespp];

	{
		BYTE* Pixels = FreeImage_GetBits(dib);
		int Width = FreeImage_GetWidth(dib);
		int Height = FreeImage_GetHeight(dib);
		int pitch = FreeImage_GetPitch(dib);
		for (int y = 0; y < f->Height + ExtraBorderExtract * 2; y++)
			for (int x = 0; x < f->Width + ExtraBorderExtract * 2; x++)
			{
				Shape[y * pitchShape + x * Bytespp + 0] = Pixels[(f->y + y - ExtraBorderExtract) * pitch + (f->x + x - ExtraBorderExtract) * Bytespp + 0];
				Shape[y * pitchShape + x * Bytespp + 1] = Pixels[(f->y + y - ExtraBorderExtract) * pitch + (f->x + x - ExtraBorderExtract) * Bytespp + 1];
				Shape[y * pitchShape + x * Bytespp + 2] = Pixels[(f->y + y - ExtraBorderExtract) * pitch + (f->x + x - ExtraBorderExtract) * Bytespp + 2];
			}
	}

	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		FontImg* i = (*itr);

		//try to not compare an "i" with an "m"
		if (abs(i->Width - f->Width) > ExtraBorderExtract || abs(i->Height - f->Height) > ExtraBorderExtract)
//		if (abs(i->Height - f->Height - 2) > 2)
			continue;

		int MinWidth = MIN(i->Width, f->Width + ExtraBorderExtract * 2);
		int MinHeight = MIN(i->Height, f->Height + ExtraBorderExtract * 2);
		__int64 SADNow = 0x00FFFFFFFFFFFF;
		int BestY, BestX;
		for (int ty = 0; ty < ExtraBorderExtract * 2; ty++)
			for (int tx = 0; tx < ExtraBorderExtract * 2; tx++)
			{
				if (i->Height + ty >= f->Height + ExtraBorderExtract * 2)
					continue;
				if (i->Width + tx >= f->Width + ExtraBorderExtract * 2)
					continue;
//				__int64 SAD = Img_SAD(i->Pixels, MinWidth, MinHeight, i->pitch, Shape, tx, ty, pitchShape);
//				__int64 SAD = Img_SAD_FontPresent(i->Pixels, MinWidth, MinHeight, i->pitch, Shape, tx, ty, pitchShape);
				BYTE* Shape1 = &Shape[ty * pitchShape + tx * Bytespp];
				__int64 SAD = Img_SAD_SQ(i->Pixels, MinWidth, MinHeight, i->pitch, Shape1, pitchShape);
				if (SAD < SADNow)
				{
					BestY = ty;
					BestX = tx;
					SADNow = (__int64)sqrt(10000*SAD);
				}
			}

		//get the morph distance from 1 image to the next
//		float PixelCountIncrease = (float)(i->Width * i->Height) / (float)(f->Width * f->Height);
//		__int64 NormalizedSAD = SADNow * 1000 / (PixelCountIncrease * PixelCountIncrease);
		__int64 PixelCountIncrease = ((i->Width * i->Height) - (f->Width * f->Height)) * 3 * 255;
		__int64 NormalizedSAD = SADNow - PixelCountIncrease;
//		__int64 NormalizedSAD = SADNow * 1000 / (i->Width * i->Height);
#if defined( _DEBUG ) && 1
		//merge the 2
		if (NormalizedSAD < ret->SAD)
		{
			int MaxWidth = MAX(i->Width + ExtraBorderExtract * 2, f->Width+ ExtraBorderExtract * 2);
			int MaxHeight = MAX(i->Height + ExtraBorderExtract * 2, f->Height+ ExtraBorderExtract * 2);

			BYTE* timg = (BYTE*)malloc((MaxWidth * 3 + 2) * MaxHeight * Bytespp);
			int tpitch = (MaxWidth * 3 + 2) * Bytespp;
			//set a background
			for (int ty = 0; ty < MaxHeight; ty++)
				for (int tx = 0; tx < MaxWidth * 3 + 2; tx++)
					timg[ty * tpitch + tx * 3 + 1] = 255;
			//split the 2 images
			for (int ty = 0; ty < MaxHeight; ty++)
			{
				timg[ty * tpitch + MaxWidth * 3 + 0] = 255;
				timg[ty * tpitch + (MaxWidth * 2 + 1) * 3 + 0] = 255;
			}
			//put the images side by side
			for (int ty = 0; ty < i->Height; ty++)
				for (int tx = 0; tx < i->Width; tx++)
				{
					timg[(ty + BestY) * tpitch + (tx + BestX) * 3 + 0] = i->Pixels[ty * i->pitch + tx * 3 + 0];
					timg[(ty + BestY) * tpitch + (tx + BestX) * 3 + 1] = i->Pixels[ty * i->pitch + tx * 3 + 1];
					timg[(ty + BestY) * tpitch + (tx + BestX) * 3 + 2] = i->Pixels[ty * i->pitch + tx * 3 + 2];
				}
			for (int ty = 0; ty < f->Height + ExtraBorderExtract * 2; ty++)
				for (int tx = 0; tx < f->Width + ExtraBorderExtract * 2; tx++)
				{
					timg[ty * tpitch + (tx + 1 + MaxWidth) * 3 + 0] = Shape[ty * pitchShape + tx * 3 + 0];
					timg[ty * tpitch + (tx + 1 + MaxWidth) * 3 + 1] = Shape[ty * pitchShape + tx * 3 + 1];
					timg[ty * tpitch + (tx + 1 + MaxWidth) * 3 + 2] = Shape[ty * pitchShape + tx * 3 + 2];
				}
			//gen the SAD map
			BYTE* SADMAP = (BYTE*)malloc((f->Height + ExtraBorderExtract * 2) * pitchShape);
			memset(SADMAP, 128, (f->Height + ExtraBorderExtract * 2) * pitchShape);
			for (int ty = 0; ty < MinHeight; ty++)
				for (int tx = 0; tx < MinWidth; tx++)
				{
					int Bdiff = abs((int)i->Pixels[ty * i->pitch + tx * Bytespp + 0] - (int)Shape[(ty + BestY) * pitchShape + (tx + BestX) * Bytespp + 0]);
					int Gdiff = abs((int)i->Pixels[ty * i->pitch + tx * Bytespp + 1] - (int)Shape[(ty + BestY) * pitchShape + (tx + BestX) * Bytespp + 1]);
					int Rdiff = abs((int)i->Pixels[ty * i->pitch + tx * Bytespp + 2] - (int)Shape[(ty + BestY) * pitchShape + (tx + BestX) * Bytespp + 2]);
					SADMAP[(ty + BestY) * pitchShape + (tx + BestX) * Bytespp + 0] = Bdiff;
					SADMAP[(ty + BestY) * pitchShape + (tx + BestX) * Bytespp + 1] = Bdiff;
					SADMAP[(ty + BestY) * pitchShape + (tx + BestX) * Bytespp + 2] = Bdiff;
				}
			for (int ty = 0; ty < f->Height + ExtraBorderExtract * 2; ty++)
				for (int tx = 0; tx < f->Width + ExtraBorderExtract * 2; tx++)
				{
					timg[ty * tpitch + (tx + 2 + MaxWidth * 2) * 3 + 0] = SADMAP[ty * pitchShape + tx * 3 + 0];
					timg[ty * tpitch + (tx + 2 + MaxWidth * 2) * 3 + 1] = SADMAP[ty * pitchShape + tx * 3 + 1];
					timg[ty * tpitch + (tx + 2 + MaxWidth * 2) * 3 + 2] = SADMAP[ty * pitchShape + tx * 3 + 2];
				}

			char tname[500];
			sprintf_s(tname, sizeof(tname), "SADDebug/%d %09lld %s.png", f->ShapeID, NormalizedSAD, i->AssignedString);
			SaveImagePNG(timg, (MaxWidth * 3 + 2), MaxHeight, tpitch, tname);
			free(timg);
		}
#endif
		//if this is best possible match. note it
		if (NormalizedSAD < ret->SAD)
		{
			ret->SAD = NormalizedSAD;
			ret->fi = i;
		}
		//		free(ScaledShape);
	}
	free(Shape);

	return ret;
}

FontSimilarityScore* GetBestMatchedFont(FIBITMAP* dib, FontExtracted* f)
{
//	return GetBestMatchedFont_FontDownScale(f);
	return GetBestMatchedFont_ShapeUpScale(f);
	//return GetBestMatchedFont_ShapeMatchFont(f);
	//return GetBestMatchedFont_ShapeSourceMatchFont(dib, f);
}

FontSimilarityScore* GetBestMatchedFontAtArea(FIBITMAP* dib, int x, int y, int SearchAreaSize)
{
	FontSimilarityScore* ret = (FontSimilarityScore*)malloc(sizeof(FontSimilarityScore));
	memset(ret, 0, sizeof(FontSimilarityScore));
	ret->SAD = 0xFFFFFFFFFFFFFF;
	int Width = FreeImage_GetWidth(dib);
	int Height = FreeImage_GetHeight(dib);
	BYTE* Pixels = FreeImage_GetBits(dib);
	int pitch = FreeImage_GetPitch(dib);
	for (int y1 = y; y1 <= y + SearchAreaSize; y1++)
		for (int x1 = x; x1 <= x + SearchAreaSize; x1++)
		{
			if (x1 > Width || y1 > Height)
				continue;
			for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
			{
				FontImg* i = (*itr);
				if (x1 + i->Width > Width || y1 + i->Height > Height)
					continue;
				__int64 SADNow = Img_SAD(i->Pixels, i->Width, i->Height, i->pitch, Pixels, x1, y1, pitch);
				//normalize SAD to original image size
				__int64 PixelCount = i->Width * i->Height;
				__int64 NormalizedSAD = SADNow * 1000000 / PixelCount / 3;
				//the smaller the image, it should somehow receive a negative effect
				NormalizedSAD -= (i->Width * 3 + i->Height * 3) * 1000000;

				//if this is best possible match. note it
				if (NormalizedSAD < ret->SAD)
				{
					ret->SAD = NormalizedSAD;
					ret->fi = i;
					ret->AdjustedX = x1;
					ret->Adjustedy = y1;
				}
			}
		}
	return ret;
}

void OCR_ResizeFontsStaticSize(int NewWidth, int NewHeight)
{
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		if ((*itr)->LoadedFromFile != 1)
			continue;

		FontImg* i = (*itr);

		int NewPitch = NewWidth * Bytespp;

		//scale the shape to the size of the current font image
		BYTE* ScaledShape = RescaleImgSubPixel(i->Pixels, i->Width, i->Height, i->pitch, NewWidth, NewHeight);
#ifdef _DEBUG
		char FileName[500];
		sprintf_s(FileName, sizeof(FileName), "FontsRescaledDebug/%s %d_%d.png", i->AssignedString, NewWidth, NewHeight);
		SaveImagePNG(ScaledShape, NewWidth, NewHeight, NewPitch, FileName);
#endif
		
		i->Width = NewWidth;
		i->Height = NewHeight;
		i->pitch = NewPitch;
		i->Scale = (float)(i->Width * i->Height) / (float(NewWidth * NewHeight));
		i->UID = FontUIDCounter++;
		i->Pixels = ScaledShape;
	}
}

void OCR_BinarizeFonts(int ToBlackAnythingBelow)
{
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		if ((*itr)->LoadedFromFile != 1)
			continue;

		FontImg* i = (*itr);
		for (int y = 0; y < i->Height; y++)
			for (int x = 0; x < i->Width; x++)
				if (i->Pixels[y * i->pitch + x * Bytespp + 0] < ToBlackAnythingBelow
					|| i->Pixels[y * i->pitch + x * Bytespp + 1] < ToBlackAnythingBelow
					|| i->Pixels[y * i->pitch + x * Bytespp + 2] < ToBlackAnythingBelow)
				{
					i->Pixels[y * i->pitch + x * Bytespp + 0] = 0;
					i->Pixels[y * i->pitch + x * Bytespp + 1] = 0;
					i->Pixels[y * i->pitch + x * Bytespp + 2] = 0;
				}
				else
				{
					i->Pixels[y * i->pitch + x * Bytespp + 0] = 255;
					i->Pixels[y * i->pitch + x * Bytespp + 1] = 255;
					i->Pixels[y * i->pitch + x * Bytespp + 2] = 255;
				}
	}
}

//Pixels1 is the font bitmap. Black & White
//Pixels2 is the shape we want to morph into the font
void OCR_GetMorphCost(BYTE* Pixels1, int Width, int Height, int pitch, BYTE* Pixels2, int pitch2, ShapeMorphStatus* ret)
{
	BYTE* tPixels1 = (BYTE*)malloc(Height * pitch);
	memcpy(tPixels1, Pixels1, Height * pitch);
	ret->Distances = 0;
	ret->PixelsMigrated = 0;
	for (int y = 0; y < Height; y++)
		for (int x = 0; x < Width; x++)
		{
			//is this a pixel we want to migrate ?
			if (Pixels2[y * pitch2 + x * Bytespp] > 128)
				continue;
			ret->PixelsMigrated++;
			//perfect overlayed pixels ?
			if (tPixels1[y * pitch + x * Bytespp] < 128)
				continue;
			int ClosestPixel = 0x00FFFFFF;
			for (int rad = 1; rad < MAX(Height,Width); rad++)
			{
				int BestY, BestX;
				for (int Edge = -rad; Edge < rad; Edge++)
				{
					int y1, x1;
					y1 = y - rad;
					x1 = x + Edge;
					if (y1 >= 0 && x1 >= 0 && x1 < Width)
						if (tPixels1[y1 * pitch + x1 * Bytespp] < 128)
						{
							int tDist = rad + abs(Edge);
							if (tDist < ClosestPixel)
							{
								ClosestPixel = tDist;
								BestY = y1;
								BestX = x1;
							}
						}
					y1 = y + rad;
					x1 = x + Edge;
					if (y1 < Height && x1 >= 0 && x1 < Width)
						if (tPixels1[y1 * pitch + x1 * Bytespp] < 128)
						{
							int tDist = rad + abs(Edge);
							if (tDist < ClosestPixel)
							{
								ClosestPixel = tDist;
								BestY = y1;
								BestX = x1;
							}
						}
					y1 = y + Edge;
					x1 = x - rad;
					if (y1 >= 0 && y1 < Height && x1 >= 0)
						if (tPixels1[y1 * pitch + x1 * Bytespp] < 128)
						{
							int tDist = rad + abs(Edge);
							if (tDist < ClosestPixel)
							{
								ClosestPixel = tDist;
								BestY = y1;
								BestX = x1;
							}
						}
					y1 = y + Edge;
					x1 = x + rad;
					if (y1 >= 0 && y1 < Height && x1 < Width)
						if (tPixels1[y1 * pitch + x1 * Bytespp] < 128)
						{
							int tDist = rad + abs(Edge);
							if (tDist < ClosestPixel)
							{
								ClosestPixel = tDist;
								BestY = y1;
								BestX = x1;
							}
						}
				}
				if (ClosestPixel != 0x00FFFFFF)
				{
					if (BestY >= 0 && BestX >= 0 && BestY < Height && BestX < Width)
						tPixels1[BestY * pitch + BestX * Bytespp] = 255;
					else
						printf("Holy s");
					break;
				}
			}
			ret->Distances += ClosestPixel;
		}
	free(tPixels1);
}

FontImg* RotateFontBy90(FontImg* i)
{
	FontImg* i2 = (FontImg*)malloc(sizeof(FontImg));
	memset(i2, 0, sizeof(FontImg));
	i2->Angle = i->Angle + 90;
	i2->AssignedString = i->AssignedString;
	i2->Height = i->Width;
	i2->Width = i->Height;
	i2->pitch = i2->Width * 3;
	i2->Scale = 1;
	i2->Pixels = (BYTE*)malloc(i2->Height * i2->pitch);
	for (int y = 0; y < i->Height; y++)
		for (int x = 0; x < i->Width; x++)
		{
			i2->Pixels[x * i2->pitch + y * 3 + 0] = i->Pixels[y * i->pitch + x * 3 + 0];
			i2->Pixels[x * i2->pitch + y * 3 + 1] = i->Pixels[y * i->pitch + x * 3 + 1];
			i2->Pixels[x * i2->pitch + y * 3 + 2] = i->Pixels[y * i->pitch + x * 3 + 2];
		}
	return i2;
}

void OCR_GenRotatedFonts()
{
	for (auto itr = CachedFonts.begin(); itr != CachedFonts.end(); itr++)
	{
		if ((*itr)->LoadedFromFile != 1)
			continue;

		//rotate 90 degree to left
		FontImg* i = (*itr);
		FontImg* i2 = RotateFontBy90(i);
		FontImg* i3 = RotateFontBy90(i2);
		FontImg* i4 = RotateFontBy90(i3);
		CachedFonts.push_back(i2);
		CachedFonts.push_back(i3);
		CachedFonts.push_back(i4);
	}
}