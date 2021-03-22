#include "source.h"
#include "wind.h"
#include <intrin.h>

#define FLOAT_PRECISSION_BITS	8

void nv12_bilinear_scale_with_crop_with_map_Y(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	size_t x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	size_t y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	size_t tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		size_t dy = tmpy >> FLOAT_PRECISSION_BITS;
		size_t y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		size_t yf = (1 << FLOAT_PRECISSION_BITS) - y;
		tmpy += y_ratio;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;
		size_t tmpx = indexBase << FLOAT_PRECISSION_BITS;

		unsigned char* tdst = &dst[i * dstWidth];

		for (int j = 0; j < (dstWidth - 1); j += 1)
		{
			size_t index = tmpx >> FLOAT_PRECISSION_BITS;
			size_t x = tmpx & ((1 << FLOAT_PRECISSION_BITS) - 1);
			size_t xf = (1 << FLOAT_PRECISSION_BITS) - x;
			tmpx += x_ratio;

			size_t srcInd[4];
			srcInd[0] = *(map_all + index);
			srcInd[1] = *(map_all + index + 1);
			srcInd[2] = *(map_all + index + srcWidth1);
			srcInd[3] = *(map_all + index + srcWidth1 + 1);

			size_t srct[4];
			srct[0] = src[srcInd[0]];
			srct[1] = src[srcInd[1]];
			srct[2] = src[srcInd[2]];
			srct[3] = src[srcInd[3]];

			size_t coeff[4];
			coeff[0] = xf * yf;
			coeff[1] = x * yf;
			coeff[2] = xf * y;
			coeff[3] = x * y;

			*tdst = (coeff[0] * srct[0]
				+ coeff[1] * srct[1]
				+ coeff[2] * srct[2]
				+ coeff[3] * srct[3]) >> 16;
			tdst++;
		}
	}
}

void nv12_bilinear_scale_with_crop_with_map_UV(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	size_t x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	size_t y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int srcuvWidth1 = srcWidth1 >> 1;
	int srcuvHeight1 = srcHeight1 >> 1;
	int dstuvWidth = dstWidth >> 1;
	int dstuvHeight = dstHeight >> 1;
	int srcoffset = srcHeight1 * srcWidth1;
	int dstoffset = dstHeight * dstWidth;

	size_t tmpy = 0;
	for (int i = 0; i < dstuvHeight - 1; i++) 
	{
		size_t dy = 2 * (tmpy >> FLOAT_PRECISSION_BITS);
		size_t y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		size_t yf = (1 << FLOAT_PRECISSION_BITS) - y;
		tmpy += y_ratio;
			
		unsigned char* tdst = &dst[dstoffset+ i * dstWidth];

		size_t indexBase = srcoffset + (dy + cropY) / 2 * srcWidth1 + cropX;
		int* map_all2 = &map_all[indexBase];

		size_t tmpx = 0;
		for (int j = 0; j < dstuvWidth - 1; j++)
		{
			size_t index = 2 * (tmpx >> FLOAT_PRECISSION_BITS);
			size_t x = tmpx & ((1 << FLOAT_PRECISSION_BITS) - 1);
			size_t xf = (1 << FLOAT_PRECISSION_BITS) - x;
			tmpx += x_ratio;

			size_t k[4];
			k[0] = xf * yf;
			k[1] = x * yf;
			k[2] = xf * y;
			k[3] = x * y;

			size_t d1 = (k[0] * src[*(map_all2 + index)] +
				k[1] * src[*(map_all2 + index + 2)] +
				k[2] * src[*(map_all2 + index + srcWidth1)] +
				k[3] * src[*(map_all2 + index + srcWidth1 + 2)]) >> 16;

			size_t d2 = (k[0] * src[*(map_all2 + index + 1)] +
				k[1] * src[*(map_all2 + index + 3)] +
				k[2] * src[*(map_all2 + index + srcWidth1 + 1)] +
				k[3] * src[*(map_all2 + index + srcWidth1 + 3)]) >> 8;

			*(size_t*)tdst = d1 | (d2 & 0xFF00);

			tdst += 2;
		}
	}
}

#if 0

//Time elapsed with crop before resize, 72412
//Time elapsed for built - in crop, 78366
void nv12_bilinear_scale_with_crop_with_map_Y_v7(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;

		size_t indexBase = ((dy + cropY) * srcWidth1 + cropX);
		size_t indexBase2 = indexBase << FLOAT_PRECISSION_BITS;
		__m128i XCounter = _mm_setr_epi32(x_ratio * 0 + indexBase2, x_ratio * 1 + indexBase2, x_ratio * 2 + indexBase2, x_ratio * 3 + indexBase2);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));

		for (int j = 0; j < (dstWidth - 1); j += 4)
		{
			__m128i xvt = _mm_and_si128(XCounter, XMask);
			__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
			XCounter = _mm_add_epi32(XCounter, XIncr);

			int res = 0;

#define CalcOnePixel(PixelInd) {\
				size_t srct[4]; \
				srct[0] = src[*(map_all + dxv.m128i_u32[PixelInd])]; \
				srct[1] = src[*(map_all + dxv.m128i_u32[PixelInd] + 1)]; \
				srct[2] = src[*(map_all + dxv.m128i_u32[PixelInd] + srcWidth1)]; \
				srct[3] = src[*(map_all + dxv.m128i_u32[PixelInd] + srcWidth1 + 1)]; \
				size_t coeff[4]; \
				coeff[0] = (0x100 - xvt.m128i_u32[PixelInd]) * (0x100 - y); \
				coeff[1] = xvt.m128i_u32[PixelInd] * (0x100 - y); \
				coeff[2] = (0x100 - xvt.m128i_u32[PixelInd]) * y; \
				coeff[3] = xvt.m128i_u32[PixelInd] * y; \
				size_t tres = (coeff[0] * srct[0] + coeff[1] * srct[1] + coeff[2] * srct[2] + coeff[3] * srct[3]) >> 16; \
				res |= (tres << (PixelInd * 8)); }

			CalcOnePixel(0);
			CalcOnePixel(1);
			CalcOnePixel(2);
			CalcOnePixel(3);
			*(int*)&dst[i * dstWidth + j] = res;
#if 0
			int t1 = (coeff[0] * srct[0] + coeff[1] * srct[1] + coeff[2] * srct[2] + coeff[3] * srct[3]) >> 16;
			int t2 = (coeffs[0][0] * srcRow1[0] + coeffs[1][0] * srcRow1[1] + coeffs[2][0] * srcRow2[0] + coeffs[3][0] * srcRow2[1]) >> 16;
			if (t1 != t2)
				t1 = t2;
#endif
		}
	}
}

void nv12_bilinear_scale_with_crop_with_map_Y_v6(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;

		size_t indexBase = ((dy + cropY) * srcWidth1 + cropX);
		size_t indexBase2 = indexBase << FLOAT_PRECISSION_BITS;
		__m128i XCounter = _mm_setr_epi32(x_ratio * 0 + indexBase2, x_ratio * 1 + indexBase2, x_ratio * 2 + indexBase2, x_ratio * 3 + indexBase2);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));

		for (int j = 0; j < (dstWidth - 1); j += 4)
		{
			__declspec(align(16)) int xv[4];
			{
				__m128i xvt = _mm_and_si128(XCounter, XMask);
				_mm_store_si128(xv, xvt);
			}
			__declspec(align(16)) int Indexpv[4];
			{
				__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
				_mm_store_si128(Indexpv, dxv);
			}
			XCounter = _mm_add_epi32(XCounter, XIncr);

			int res = 0;

#define CalcOnePixel(PixelInd) {\
				size_t srcInd[4]; \
				srcInd[0] = *(map_all + Indexpv[PixelInd]); \
				srcInd[1] = *(map_all + Indexpv[PixelInd] + 1); \
				srcInd[2] = *(map_all + Indexpv[PixelInd] + srcWidth1); \
				srcInd[3] = *(map_all + Indexpv[PixelInd] + srcWidth1 + 1); \
				size_t srct[4]; \
				srct[0] = src[srcInd[0]]; \
				srct[1] = src[srcInd[1]]; \
				srct[2] = src[srcInd[2]]; \
				srct[3] = src[srcInd[3]]; \
				size_t coeff[4]; \
				coeff[0] = (0x100 - xv[PixelInd]) * (0x100 - y); \
				coeff[1] = xv[PixelInd] * (0x100 - y); \
				coeff[2] = (0x100 - xv[PixelInd]) * y; \
				coeff[3] = xv[PixelInd] * y; \
				size_t tres = (coeff[0] * srct[0] + coeff[1] * srct[1] + coeff[2] * srct[2] + coeff[3] * srct[3]) >> 16; \
				res |= (tres << (PixelInd * 8)); }

			CalcOnePixel(0);
			CalcOnePixel(1);
			CalcOnePixel(2);
			CalcOnePixel(3);
			*(int*)&dst[i * dstWidth + j] = res;
#if 0
			int t1 = (coeff[0] * srct[0] + coeff[1] * srct[1] + coeff[2] * srct[2] + coeff[3] * srct[3]) >> 16;
			int t2 = (coeffs[0][0] * srcRow1[0] + coeffs[1][0] * srcRow1[1] + coeffs[2][0] * srcRow2[0] + coeffs[3][0] * srcRow2[1]) >> 16;
			if (t1 != t2)
				t1 = t2;
#endif
		}
	}
}
//Time elapsed with crop before resize, 63754
//Time elapsed for built - in crop, 123652
void nv12_bilinear_scale_with_crop_with_map_Y_v5(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;
		int tmpx = 0;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;

		__m128i XCounter = _mm_setr_epi32(x_ratio * 0, x_ratio * 1, x_ratio * 2, x_ratio * 3);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i yv = _mm_set1_epi32(y);
		__m128i myv = _mm_set1_epi32((1 << FLOAT_PRECISSION_BITS) - y);

		for (int j = 0; j < (dstWidth - 1 + 3); j += 4)
		{
			__m128i coeffsv[4];
			{
				__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));
				__m128i xv = _mm_and_si128(XCounter, XMask);
				__m128i MirrorMask = _mm_set1_epi32(1 << FLOAT_PRECISSION_BITS);
				__m128i mxv = _mm_sub_epi32(MirrorMask, xv);
				coeffsv[0] = _mm_mullo_epi32(mxv, myv);
				coeffsv[1] = _mm_mullo_epi32(xv, myv);
				coeffsv[2] = _mm_mullo_epi32(mxv, yv);
				coeffsv[3] = _mm_mullo_epi32(xv, yv);
			}

			__declspec(align(16)) int Indexpv[4];
			{
				__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
				__m128i IndexBaseV = _mm_set1_epi32(indexBase);
				__m128i indexv = _mm_add_epi32(dxv, IndexBaseV);
				_mm_store_si128(&Indexpv[0], indexv);
			}

			XCounter = _mm_add_epi32(XCounter, XIncr);

			__m128i srcRow1v[2];
			__m128i srcRow2v[2];
			{
				__declspec(align(16)) unsigned int srcRow1[4][2];
				__declspec(align(16)) unsigned int srcRow2[4][2];
				for (int ind = 0; ind < 4; ind++)
				{
					srcRow1[0][ind] = src[*(map_all + Indexpv[ind] + 0)];
					srcRow1[1][ind] = src[*(map_all + Indexpv[ind] + 1)];
					srcRow2[0][ind] = src[*(map_all + Indexpv[ind] + srcWidth1 + 0)];
					srcRow2[1][ind] = src[*(map_all + Indexpv[ind] + srcWidth1 + 1)];
				}

				srcRow1v[0] = _mm_load_si128(&srcRow1[0][0]);
				srcRow1v[1] = _mm_load_si128(&srcRow1[1][0]);
				srcRow2v[0] = _mm_load_si128(&srcRow2[0][0]);
				srcRow2v[1] = _mm_load_si128(&srcRow2[1][0]);
			}

			srcRow1v[0] = _mm_mullo_epi32(coeffsv[0], srcRow1v[0]);
			srcRow1v[1] = _mm_mullo_epi32(coeffsv[1], srcRow1v[1]);
			srcRow2v[0] = _mm_mullo_epi32(coeffsv[2], srcRow2v[0]);
			srcRow2v[1] = _mm_mullo_epi32(coeffsv[3], srcRow2v[1]);

			srcRow1v[0] = _mm_add_epi32(srcRow1v[0], srcRow1v[1]);
			srcRow2v[0] = _mm_add_epi32(srcRow2v[0], srcRow2v[1]);

			srcRow1v[0] = _mm_add_epi32(srcRow1v[0], srcRow2v[0]);

			__m128i ShuffleMask = _mm_setr_epi8(2, 6, 10, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
			__m128i dstval = _mm_shuffle_epi8(srcRow1v[0], ShuffleMask);

			_mm_storeu_si32(&dst[i * dstWidth + j + 0], dstval);
		}
	}
}

void nv12_bilinear_scale_with_crop_with_map_Y_v4(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;
		int tmpx = 0;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;

		__m128i XCounter = _mm_setr_epi32(x_ratio * 0, x_ratio * 1, x_ratio * 2, x_ratio * 3);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));
		__m128i IndexBaseV = _mm_set1_epi32(indexBase);
		__m128i MirrorMask = _mm_set1_epi32(1 << FLOAT_PRECISSION_BITS);
		__m128i yv = _mm_set1_epi32(y);
		__m128i myv = _mm_set1_epi32((1 << FLOAT_PRECISSION_BITS) - y);
		__m128i ShuffleMask = _mm_setr_epi8(2, 6, 10, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);

		for (int j = 0; j < (dstWidth - 1 + 3); j += 4)
		{
			__m128i coeffsv[4];
			{
				__m128i xv = _mm_and_si128(XCounter, XMask);
				__m128i mxv = _mm_sub_epi32(MirrorMask, xv);
				coeffsv[0] = _mm_mullo_epi32(mxv, myv);
				coeffsv[1] = _mm_mullo_epi32(xv, myv);
				coeffsv[2] = _mm_mullo_epi32(mxv, yv);
				coeffsv[3] = _mm_mullo_epi32(xv, yv);
			}

			__declspec(align(16)) int Indexpv[4];
			{
				__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
				__m128i indexv = _mm_add_epi32(dxv, IndexBaseV);
				_mm_store_si128(&Indexpv[0], indexv);
			}

			XCounter = _mm_add_epi32(XCounter, XIncr);

			__m128i srcRow1v[2];
			__m128i srcRow2v[2];
			{
				__declspec(align(16)) unsigned int srcRow1[4][2];
				__declspec(align(16)) unsigned int srcRow2[4][2];
				for (int ind = 0; ind < 4; ind++)
				{
					srcRow1[0][ind] = src[*(map_all + Indexpv[ind] + 0)];
					srcRow1[1][ind] = src[*(map_all + Indexpv[ind] + 1)];
					srcRow2[0][ind] = src[*(map_all + Indexpv[ind] + srcWidth1 + 0)];
					srcRow2[1][ind] = src[*(map_all + Indexpv[ind] + srcWidth1 + 1)];
				}

				srcRow1v[0] = _mm_load_si128(&srcRow1[0][0]);
				srcRow1v[1] = _mm_load_si128(&srcRow1[1][0]);
				srcRow2v[0] = _mm_load_si128(&srcRow2[0][0]);
				srcRow2v[1] = _mm_load_si128(&srcRow2[1][0]);
			}

			srcRow1v[0] = _mm_mullo_epi32(coeffsv[0], srcRow1v[0]);
			srcRow1v[1] = _mm_mullo_epi32(coeffsv[1], srcRow1v[1]);
			srcRow2v[0] = _mm_mullo_epi32(coeffsv[2], srcRow2v[0]);
			srcRow2v[1] = _mm_mullo_epi32(coeffsv[3], srcRow2v[1]);

			srcRow1v[0] = _mm_add_epi32(srcRow1v[0], srcRow1v[1]);
			srcRow2v[0] = _mm_add_epi32(srcRow2v[0], srcRow2v[1]);

			srcRow1v[0] = _mm_add_epi32(srcRow1v[0], srcRow2v[0]);

			__m128i dstval = _mm_shuffle_epi8(srcRow1v[0], ShuffleMask);

			_mm_storeu_si32(&dst[i * dstWidth + j + 0], dstval);
			/*
						__declspec(align(16)) int coeffs[4][4];
						_mm_store_si128(&coeffs[0][0], coeffsv[0]);
						_mm_store_si128(&coeffs[1][0], coeffsv[1]);
						_mm_store_si128(&coeffs[2][0], coeffsv[2]);
						_mm_store_si128(&coeffs[3][0], coeffsv[3]);

						dst[i * dstWidth + j + 0] = (coeffs[0][0] * srcRow1[0][0]
							+ coeffs[1][0] * srcRow1[1][0]
							+ coeffs[2][0] * srcRow2[0][0]
							+ coeffs[3][0] * srcRow2[1][0]) >> 16;
						dst[i * dstWidth + j + 1] = (coeffs[0][1] * srcRow1[0][1]
							+ coeffs[1][1] * srcRow1[1][1]
							+ coeffs[2][1] * srcRow2[0][1]
							+ coeffs[3][1] * srcRow2[1][1]) >> 16;
						dst[i * dstWidth + j + 2] = (coeffs[0][2] * srcRow1[0][2]
							+ coeffs[1][2] * srcRow1[1][2]
							+ coeffs[2][2] * srcRow2[0][2]
							+ coeffs[3][2] * srcRow2[1][2]) >> 16;
						dst[i * dstWidth + j + 3] = (coeffs[0][3] * srcRow1[0][3]
							+ coeffs[1][3] * srcRow1[1][3]
							+ coeffs[2][3] * srcRow2[0][3]
							+ coeffs[3][3] * srcRow2[1][3]) >> 16;

						if (dstval.m128i_u8[0] != dst[i * dstWidth + j + 0])
							dst[i * dstWidth + j + 0] = 0;
						if (dstval.m128i_u8[1] != dst[i * dstWidth + j + 1])
							dst[i * dstWidth + j + 0] = 0;
						if (dstval.m128i_u8[2] != dst[i * dstWidth + j + 2])
							dst[i * dstWidth + j + 0] = 0;
						if (dstval.m128i_u8[3] != dst[i * dstWidth + j + 3])
							dst[i * dstWidth + j + 0] = 0; /**/
		}
	}
}

void nv12_bilinear_scale_with_crop_with_map_Y_v3(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;
		int tmpx = 0;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;

		__m128i XCounter = _mm_setr_epi32(x_ratio * 0, x_ratio * 1, x_ratio * 2, x_ratio * 3);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));
		__m128i IndexBaseV = _mm_set1_epi32(indexBase);
		__m128i MirrorMask = _mm_set1_epi32(1 << FLOAT_PRECISSION_BITS);
		__m128i yv = _mm_set1_epi32(y);
		__m128i myv = _mm_set1_epi32((1 << FLOAT_PRECISSION_BITS) - y);

		for (int j = 0; j < (dstWidth - 1); j += 4)
		{
			__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
			__m128i xv = _mm_and_si128(XCounter, XMask);
			XCounter = _mm_add_epi32(XCounter, XIncr);

			__m128i indexv = _mm_add_epi32(dxv, IndexBaseV);
			__declspec(align(16)) int Indexpv[4];
			_mm_store_si128(&Indexpv[0], indexv);

			__declspec(align(16)) unsigned int srcRow1[4][2];
			__declspec(align(16)) unsigned int srcRow2[4][2];
			for (int ind = 0; ind < 4; ind++)
			{
				srcRow1[ind][0] = src[*(map_all + Indexpv[ind] + 0)];
				srcRow1[ind][1] = src[*(map_all + Indexpv[ind] + 1)];
				srcRow2[ind][0] = src[*(map_all + Indexpv[ind] + srcWidth1 + 0)];
				srcRow2[ind][1] = src[*(map_all + Indexpv[ind] + srcWidth1 + 1)];
			}

			__m128i mxv = _mm_sub_epi32(MirrorMask, xv);
			__m128i coeffsv[4];
			coeffsv[0] = _mm_mullo_epi32(mxv, myv);
			coeffsv[1] = _mm_mullo_epi32(xv, myv);
			coeffsv[2] = _mm_mullo_epi32(mxv, yv);
			coeffsv[3] = _mm_mullo_epi32(xv, yv);

			//			_mm_mullo_epi32(coeffsv[0], srcRow1[0][0]);

			__declspec(align(16)) int coeffs[4][4];
			_mm_store_si128(&coeffs[0][0], coeffsv[0]);
			_mm_store_si128(&coeffs[1][0], coeffsv[1]);
			_mm_store_si128(&coeffs[2][0], coeffsv[2]);
			_mm_store_si128(&coeffs[3][0], coeffsv[3]);

			dst[i * dstWidth + j + 0] = (coeffs[0][0] * srcRow1[0][0]
				+ coeffs[1][0] * srcRow1[0][1]
				+ coeffs[2][0] * srcRow2[0][0]
				+ coeffs[3][0] * srcRow2[0][1]) >> 16;
			dst[i * dstWidth + j + 1] = (coeffs[0][1] * srcRow1[1][0]
				+ coeffs[1][1] * srcRow1[1][1]
				+ coeffs[2][1] * srcRow2[1][0]
				+ coeffs[3][1] * srcRow2[1][1]) >> 16;
			dst[i * dstWidth + j + 2] = (coeffs[0][2] * srcRow1[2][0]
				+ coeffs[1][2] * srcRow1[2][1]
				+ coeffs[2][2] * srcRow2[2][0]
				+ coeffs[3][2] * srcRow2[2][1]) >> 16;
			dst[i * dstWidth + j + 3] = (coeffs[0][3] * srcRow1[3][0]
				+ coeffs[1][3] * srcRow1[3][1]
				+ coeffs[2][3] * srcRow2[3][0]
				+ coeffs[3][3] * srcRow2[3][1]) >> 16;
		}
	}
}

void nv12_bilinear_scale_with_crop_with_map_Y_v2(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;
		int tmpx = 0;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;

		__m128i XCounter = _mm_setr_epi32(x_ratio * 0, x_ratio * 1, x_ratio * 2, x_ratio * 3);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));
		__m128i IndexBaseV = _mm_set1_epi32(indexBase);
		__m128i MirrorMask = _mm_set1_epi32(1 << FLOAT_PRECISSION_BITS);
		__m128i yv = _mm_set1_epi32(y);
		__m128i myv = _mm_set1_epi32((1 << FLOAT_PRECISSION_BITS) - y);

		for (int j = 0; j < (dstWidth - 1); j += 4)
		{
			__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
			__m128i xv = _mm_and_si128(XCounter, XMask);
			XCounter = _mm_add_epi32(XCounter, XIncr);
			__m128i indexv = _mm_add_epi32(dxv, IndexBaseV);

			__m128i mxv = _mm_sub_epi32(MirrorMask, xv);
			__m128i coeffsv[4];
			coeffsv[0] = _mm_mullo_epi32(mxv, myv);
			coeffsv[1] = _mm_mullo_epi32(xv, myv);
			coeffsv[2] = _mm_mullo_epi32(mxv, yv);
			coeffsv[3] = _mm_mullo_epi32(xv, yv);

			__declspec(align(16)) int Indexpv[4];
			_mm_store_si128(&Indexpv[0], indexv);

			size_t srcIndRow1[8];
			size_t srcIndRow2[8];
			for (int ind = 0; ind < 4; ind++)
			{
				srcIndRow1[2 * ind + 0] = *(map_all + Indexpv[ind] + 0);
				srcIndRow1[2 * ind + 1] = *(map_all + Indexpv[ind] + 1);
				srcIndRow2[2 * ind + 0] = *(map_all + Indexpv[ind] + srcWidth1 + 0);
				srcIndRow2[2 * ind + 1] = *(map_all + Indexpv[ind] + srcWidth1 + 1);
			}
			size_t srcRow1[8];
			size_t srcRow2[8];
			for (int ind = 0; ind < 8; ind++)
			{
				srcRow1[ind] = src[srcIndRow1[ind]];
				srcRow2[ind] = src[srcIndRow2[ind]];
			}

			__declspec(align(16)) int coeffs[4][4];
			_mm_store_si128(&coeffs[0][0], coeffsv[0]);
			_mm_store_si128(&coeffs[1][0], coeffsv[1]);
			_mm_store_si128(&coeffs[2][0], coeffsv[2]);
			_mm_store_si128(&coeffs[3][0], coeffsv[3]);

			dst[i * dstWidth + j + 0] = (coeffs[0][0] * srcRow1[0]
				+ coeffs[1][0] * srcRow1[1]
				+ coeffs[2][0] * srcRow2[0]
				+ coeffs[3][0] * srcRow2[1]) >> 16;
			dst[i * dstWidth + j + 1] = (coeffs[0][1] * srcRow1[2]
				+ coeffs[1][1] * srcRow1[3]
				+ coeffs[2][1] * srcRow2[2]
				+ coeffs[3][1] * srcRow2[3]) >> 16;
			dst[i * dstWidth + j + 2] = (coeffs[0][2] * srcRow1[4]
				+ coeffs[1][2] * srcRow1[5]
				+ coeffs[2][2] * srcRow2[4]
				+ coeffs[3][2] * srcRow2[5]) >> 16;
			dst[i * dstWidth + j + 3] = (coeffs[0][3] * srcRow1[6]
				+ coeffs[1][3] * srcRow1[7]
				+ coeffs[2][3] * srcRow2[6]
				+ coeffs[3][3] * srcRow2[7]) >> 16;
		}
	}
}


void nv12_bilinear_scale_with_crop_with_map_Y_v1(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++) 
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1<< FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;
		int tmpx = 0;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;

		__m128i XCounter = _mm_setr_epi32(x_ratio * 0, x_ratio * 1, x_ratio * 2, x_ratio * 3);
		__m128i XIncr = _mm_set1_epi32(x_ratio * 4);
		__m128i XMask = _mm_set1_epi32(((1 << FLOAT_PRECISSION_BITS) - 1));
		__m128i IndexBaseV = _mm_set1_epi32(indexBase);
		__m128i MirrorMask = _mm_set1_epi32(1 << FLOAT_PRECISSION_BITS);
		__m128i yv = _mm_set1_epi32(y);
		__m128i myv = _mm_set1_epi32((1 << FLOAT_PRECISSION_BITS) - y);

//		for (int j = 0; j < (dstWidth - 1); j+=1)
		for (int j = 0; j < (dstWidth - 1); j+=4)
		{
			__m128i dxv = _mm_srli_epi32(XCounter, FLOAT_PRECISSION_BITS);
			__m128i xv = _mm_and_si128(XCounter, XMask);
			XCounter = _mm_add_epi32(XCounter, XIncr);
			__m128i indexv = _mm_add_epi32(dxv, IndexBaseV);

			__m128i mxv = _mm_sub_epi32(MirrorMask, xv);
			__m128i coeffsv[4];
			coeffsv[0] = _mm_mullo_epi32(mxv, myv);
			coeffsv[1] = _mm_mullo_epi32(xv, myv);
			coeffsv[2] = _mm_mullo_epi32(mxv, yv);
			coeffsv[3] = _mm_mullo_epi32(xv, yv);

			__declspec(align(16)) int Indexpv[4];
			_mm_store_si128(&Indexpv[0], indexv);

			size_t srcIndRow1[8];
			size_t srcIndRow2[8];
			for (int ind = 0; ind < 4; ind++)
			{
				srcIndRow1[2 * ind + 0] = *(map_all + Indexpv[ind] + 0);
				srcIndRow1[2 * ind + 1] = *(map_all + Indexpv[ind] + 1);
				srcIndRow2[2 * ind + 0] = *(map_all + Indexpv[ind] + srcWidth1 + 0);
				srcIndRow2[2 * ind + 1] = *(map_all + Indexpv[ind] + srcWidth1 + 1);
			}
			unsigned char srcRow1[8];
			unsigned char srcRow2[8];
			for (int ind = 0; ind < 8; ind++)
			{
				srcRow1[ind] = src[srcIndRow1[ind]];
				srcRow2[ind] = src[srcIndRow2[ind]];
			}

			__declspec(align(16)) int coeffs[4][4];
			_mm_store_si128(&coeffs[0][0], coeffsv[0]);
			_mm_store_si128(&coeffs[1][0], coeffsv[1]);
			_mm_store_si128(&coeffs[2][0], coeffsv[2]);
			_mm_store_si128(&coeffs[3][0], coeffsv[3]);

			dst[i * dstWidth + j + 0] = (coeffs[0][0] * srcRow1[0]
				+ coeffs[1][0] * srcRow1[1]
				+ coeffs[2][0] * srcRow2[0]
				+ coeffs[3][0] * srcRow2[1]) >> 16;
			dst[i * dstWidth + j + 1] = (coeffs[0][1] * srcRow1[2]
				+ coeffs[1][1] * srcRow1[3]
				+ coeffs[2][1] * srcRow2[2]
				+ coeffs[3][1] * srcRow2[3]) >> 16;
			dst[i * dstWidth + j + 2] = (coeffs[0][2] * srcRow1[4]
				+ coeffs[1][2] * srcRow1[5]
				+ coeffs[2][2] * srcRow2[4]
				+ coeffs[3][2] * srcRow2[5]) >> 16;
			dst[i * dstWidth + j + 3] = (coeffs[0][3] * srcRow1[6]
				+ coeffs[1][3] * srcRow1[7]
				+ coeffs[2][3] * srcRow2[6]
				+ coeffs[3][3] * srcRow2[7]) >> 16;

/*			int dx = tmpx >> FLOAT_PRECISSION_BITS;
			int x = tmpx & ((1 << FLOAT_PRECISSION_BITS) - 1);
			tmpx += x_ratio;

			int index = indexBase + dx;

			size_t srcInd[4];
			srcInd[0] = *(map_all + index);
			srcInd[1] = *(map_all + index + 1);
			srcInd[2] = *(map_all + index + srcWidth1);
			srcInd[3] = *(map_all + index + srcWidth1 + 1);

			size_t srct[4];
			srct[0] = src[srcInd[0]];
			srct[1] = src[srcInd[1]];
			srct[2] = src[srcInd[2]];
			srct[3] = src[srcInd[3]];

			size_t coeff[4];
			coeff[0] = (0x100 - x) * (0x100 - y);
			coeff[1] = x * (0x100 - y);
			coeff[2] = (0x100 - x) * y;
			coeff[3] = x * y;

			dst[i * dstWidth + j] = (coeff[0] * srct[0]
				+ coeff[1] * srct[1]
				+ coeff[2] * srct[2]
				+ coeff[3] * srct[3]) >> 16; /**/
#if 0
			int t1 = (coeff[0] * srct[0] + coeff[1] * srct[1] + coeff[2] * srct[2] + coeff[3] * srct[3]) >> 16;
			int t2 = (coeffs[0][0] * srcRow1[0] + coeffs[1][0] * srcRow1[1] + coeffs[2][0] * srcRow2[0] + coeffs[3][0] * srcRow2[1]) >> 16;
			if (t1 != t2)
				t1 = t2;
#endif
		}
	}
}

void nv12_bilinear_scale_with_crop_with_map_Y_v0(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x_ratio = (cropWidth << FLOAT_PRECISSION_BITS) / dstWidth;
	int y_ratio = (cropHeight << FLOAT_PRECISSION_BITS) / dstHeight;

	int tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++)
	{
		int dy = tmpy >> FLOAT_PRECISSION_BITS;
		int y = tmpy & ((1 << FLOAT_PRECISSION_BITS) - 1);
		tmpy += y_ratio;
		int tmpx = 0;

		size_t indexBase = (dy + cropY) * srcWidth1 + cropX;

		for (int j = 0; j < (dstWidth - 1); j += 1)
		{
			int dx = tmpx >> FLOAT_PRECISSION_BITS;
			int x = tmpx & ((1 << FLOAT_PRECISSION_BITS) - 1);
			tmpx += x_ratio;

			int index = indexBase + dx;

			size_t srcInd[4];
			srcInd[0] = *(map_all + index);
			srcInd[1] = *(map_all + index + 1);
			srcInd[2] = *(map_all + index + srcWidth1);
			srcInd[3] = *(map_all + index + srcWidth1 + 1);

			size_t srct[4];
			srct[0] = src[srcInd[0]];
			srct[1] = src[srcInd[1]];
			srct[2] = src[srcInd[2]];
			srct[3] = src[srcInd[3]];

			size_t coeff[4];
			coeff[0] = (0x100 - x) * (0x100 - y);
			coeff[1] = x * (0x100 - y);
			coeff[2] = (0x100 - x) * y;
			coeff[3] = x * y;

			dst[i * dstWidth + j] = (coeff[0] * srct[0]
				+ coeff[1] * srct[1]
				+ coeff[2] * srct[2]
				+ coeff[3] * srct[3]) >> 16;
#if 0
			int t1 = (coeff[0] * srct[0] + coeff[1] * srct[1] + coeff[2] * srct[2] + coeff[3] * srct[3]) >> 16;
			int t2 = (coeffs[0][0] * srcRow1[0] + coeffs[1][0] * srcRow1[1] + coeffs[2][0] * srcRow2[0] + coeffs[3][0] * srcRow2[1]) >> 16;
			if (t1 != t2)
				t1 = t2;
#endif
		}
	}
}

#endif

void nv12_bilinear_scale_with_crop_with_map_(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int* map_all)
{
	int x, y, index, tmpx, tmpy;
	int dx, dy;
	int srcWidth = cropWidth, srcHeight = cropHeight;

	int x_ratio = (srcWidth << 8) / dstWidth;
	int y_ratio = (srcHeight << 8) / dstHeight;

	int srcuvWidth1 = srcWidth1 >> 1;
	int srcuvHeight1 = srcHeight1 >> 1;
	int dstuvWidth = dstWidth >> 1;
	int dstuvHeight = dstHeight >> 1;
	int srcoffset = srcHeight1 * srcWidth1;
	int dstoffset = dstHeight * dstWidth;

	nv12_bilinear_scale_with_crop_with_map_Y(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight, map_all);

	//      Y bottom border
	tmpy = y_ratio * (dstHeight - 1);
	dy = tmpy >> 8;
	y = tmpy & 0xFF;
	tmpx = 0;
	for (int j = 0; j < dstWidth; j++) {
		dx = tmpx >> 8;
		x = tmpx & 0xFF;
		index = (dy + cropY) * srcWidth1 + dx + cropX;

		dst[(dstHeight - 1) * dstWidth + j] = src[*(map_all + index)];
		tmpx += x_ratio;
	}
	//      Y right border
	tmpx = x_ratio * (dstWidth - 1);
	dx = tmpx >> 8;
	x = tmpx & 0xFF;
	tmpy = 0;
	for (int i = 0; i < dstHeight - 1; i++) {
		dy = tmpy >> 8;
		y = tmpy & 0xFF;
		index = (dy + cropY) * srcWidth1 + dx + cropX;

		dst[i * dstWidth + (dstWidth - 1)] = src[*(map_all + index)];
		tmpy += y_ratio;
	}

	nv12_bilinear_scale_with_crop_with_map_UV(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight, map_all);

	//      UV bottom border
	tmpy = y_ratio * (dstuvHeight - 1);
	dy = 2 * (tmpy >> 8);
	y = tmpy & 0xFF;
	tmpx = 0;
	for (int j = 0; j < dstuvWidth; j++) {
		dx = 2 * (tmpx >> 8);
		x = tmpx & 0xFF;
		index = srcoffset + 2 * ((dy + cropY) / 2 * srcuvWidth1 + (dx + cropX) / 2);

		dst[dstoffset + 2 * ((dstuvHeight - 1) * dstuvWidth + j)] = src[*(map_all + index)];
		dst[dstoffset + 2 * ((dstuvHeight - 1) * dstuvWidth + j) + 1] = src[*(map_all + index + 1)];
		tmpx += x_ratio;

	}

	//      UV right border
	tmpy = 0;
	tmpx = x_ratio * (dstuvWidth - 1);
	dx = 2 * (tmpx >> 8);
	x = tmpx & 0xFF;
	for (int i = 0; i < dstuvHeight - 1; i++) {
		dy = 2 * (tmpy >> 8);
		y = tmpy & 0xFF;
		index = srcoffset + 2 * ((dy + cropY) / 2 * srcuvWidth1 + (dx + cropX) / 2);

		dst[dstoffset + 2 * (i * dstuvWidth + dstuvWidth - 1)] = src[*(map_all + index)];
		dst[dstoffset + 2 * (i * dstuvWidth + dstuvWidth - 1) + 1] = src[*(map_all + index + 1)];
		tmpy += y_ratio;
	}
}
