#include <intrin.h>
#include "Source.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define FLOAT_PRECISSION_BITS2 8
#define FLOAT_PRECISSION_BITS3 12


void nv12_bilinear_scale_with_crop_AVX_YUV_V2(uchar* src, uchar* dst, int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    size_t srcWidth = cropWidth, srcHeight = cropHeight;
    size_t x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS3) / dstWidth;
    size_t y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS3) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned int SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned int CoeffXr[5000];

    assert(dstWidth < __crt_countof(SrcMask2));
    assert(dstWidth < __crt_countof(CoeffXr));
    assert(x_ratio < ((15 << FLOAT_PRECISSION_BITS3) / 8)); // this limitation comes from register size

    size_t PrevBlockEnd = 0;
    for (size_t j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS3;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS3) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (unsigned int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned int)(x - PrevBlockEnd) | 0x80808000;
    }

    __m256i flipper = _mm256_setr_epi32(1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3, 1 << FLOAT_PRECISSION_BITS3);
    __m256i m = _mm256_setr_epi8(3, 7, 11, 15, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 19, 23, 27, 31, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS3; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS3) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS3) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (size_t j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS3;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00, srcPixels_00);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01, srcPixels_01);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_10, srcPixels_10);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11, srcPixels_11);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes : 2,6,10,14, 18,22,26,30
            __m256i r3 = _mm256_shuffle_epi8(r2, m);
            *(int*)(dst1 + 0) = r3.m256i_u32[0];
            *(int*)(dst1 + 4) = r3.m256i_u32[4];
            dst1 += 8;
        }
    }
#if 1
    PrevBlockEnd = 0;
    for (size_t j = 0; j < dstWidth; j += 2)
    {
        size_t xt = (x_ratio * j / 2);
        size_t x = 2 * (xt >> FLOAT_PRECISSION_BITS3);
        size_t x_r = (xt * 2) & ((1 << FLOAT_PRECISSION_BITS3) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j + 0] = (unsigned int)x_r;
        CoeffXr[j + 1] = (unsigned int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j + 0] = (unsigned int)(x + 0 - PrevBlockEnd) | 0x80808000;
        SrcMask2[j + 1] = (unsigned int)(x + 1 - PrevBlockEnd) | 0x80808000;
    }

    size_t srcuvWidth1 = srcWidth1 / 2;
    size_t srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    size_t dstoffset = dstHeight * dstWidth;
    uchar* dstuv = &dst[dstoffset];
    for (size_t i = 0; i < dstHeight / 2; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS3; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS3) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS3) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[(size_t)y * srcWidth1 + srcoffset];
        uchar* dst1 = &dstuv[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (size_t j = 0; j < dstWidth / 2; j += 4)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS3;
            x = x * 2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00UV = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10UV = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01UV = _mm_srli_si128(srcPixels_00UV, 2);
            __m128i srcPixels_11UV = _mm_srli_si128(srcPixels_10UV, 2);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00UV, srcPixels_00UV);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01UV, srcPixels_01UV);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_01UV, srcPixels_01UV);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11UV, srcPixels_11UV);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes : 2,6,10,14, 18,22,26,30
            __m256i r3 = _mm256_shuffle_epi8(r2, m);
            *(int*)(dst1 + 0) = r3.m256i_u32[0];
            *(int*)(dst1 + 4) = r3.m256i_u32[4];
            dst1 += 8;
        }
    }
#endif
}

void nv12_bilinear_scale_with_crop_AVX_YUV_V1(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned int SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned int CoeffXr[5000];

    assert(dstWidth < __crt_countof(SrcMask2));
    assert(dstWidth < __crt_countof(CoeffXr));
    assert(x_ratio < ((15 << FLOAT_PRECISSION_BITS2) / 8)); // this limitation comes from register size

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (unsigned int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned int)(x - PrevBlockEnd) | 0x80808000;
    }

    __m256i flipper = _mm256_setr_epi32(256, 256, 256, 256, 256, 256, 256, 256);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00, srcPixels_00);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01, srcPixels_01);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_10, srcPixels_10);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11, srcPixels_11);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes : 2,6,10,14, 18,22,26,30
            __m256i m = _mm256_setr_epi8(2, 6, 10, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 18, 22, 26, 30, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
            __m256i r3 = _mm256_shuffle_epi8(r2, m);
            *(int*)(dst1 + 0) = r3.m256i_u32[0];
            *(int*)(dst1 + 4) = r3.m256i_u32[4];
            dst1 += 8;
        }
    }
#if 1
    PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j+=2)
    {
        size_t xt = (x_ratio * j / 2);
        size_t x = 2 * (xt >> FLOAT_PRECISSION_BITS2);
        size_t x_r = (xt * 2) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j + 0] = (unsigned int)x_r;
        CoeffXr[j + 1] = (unsigned int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j + 0] = (unsigned int)(x + 0 - PrevBlockEnd) | 0x80808000;
        SrcMask2[j + 1] = (unsigned int)(x + 1 - PrevBlockEnd) | 0x80808000;
    }

    int srcuvWidth1 = srcWidth1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;
    uchar* dstuv = &dst[dstoffset];
    for (size_t i = 0; i < dstHeight / 2; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[(size_t)y * srcWidth1 + srcoffset];
        uchar* dst1 = &dstuv[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth / 2; j += 4)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
            x = x * 2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00UV = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10UV = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01UV = _mm_srli_si128(srcPixels_00UV, 2);
            __m128i srcPixels_11UV = _mm_srli_si128(srcPixels_10UV, 2);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00UV, srcPixels_00UV);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01UV, srcPixels_01UV);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_01UV, srcPixels_01UV);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11UV, srcPixels_11UV);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes : 2,6,10,14, 18,22,26,30
            __m256i m = _mm256_setr_epi8(2, 6, 10, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 18, 22, 26, 30, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
            __m256i r3 = _mm256_shuffle_epi8(r2, m);
            *(int*)(dst1 + 0) = r3.m256i_u32[0];
            *(int*)(dst1 + 4) = r3.m256i_u32[4];
            dst1 += 8;
        }
    }
#endif
}

void nv12_bilinear_scale_with_crop_AVX_Y_V7(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned int SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned int CoeffXr[5000];

    assert(dstWidth < __crt_countof(SrcMask2));
    assert(dstWidth < __crt_countof(CoeffXr));
    assert(x_ratio < ((15 << FLOAT_PRECISSION_BITS2) / 8)); // this limitation comes from register size

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (unsigned int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned int)(x - PrevBlockEnd) | 0x80808000;
    }

    __m256i flipper = _mm256_setr_epi32(256, 256, 256, 256, 256, 256, 256, 256);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00, srcPixels_00);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01, srcPixels_01);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_10, srcPixels_10);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11, srcPixels_11);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes : 2,6,10,14, 18,22,26,30
            __m256i m = _mm256_setr_epi8(2, 6, 10, 14, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 18, 22, 26, 30, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80);
            __m256i r3 = _mm256_shuffle_epi8(r2, m);
            *(int*)(dst1 + 0) = r3.m256i_u32[0];
            *(int*)(dst1 + 4) = r3.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_V6(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned int SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned int CoeffXr[5000];

    assert(dstWidth < __crt_countof(SrcMask2));
    assert(dstWidth < __crt_countof(CoeffXr));
    assert(x_ratio < ((15 << FLOAT_PRECISSION_BITS2) / 8)); // this limitation comes from register size

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (unsigned int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned int)(x - PrevBlockEnd) | 0x80808000;
    }

    __m256i flipper = _mm256_setr_epi32(256, 256, 256, 256, 256, 256, 256, 256);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00, srcPixels_00);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01, srcPixels_01);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_10, srcPixels_10);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11, srcPixels_11);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes : 2,6,10,14, 18,22,26,30
            __m256i r3 = _mm256_srli_epi32(r2, 16);
            __m256i r4 = _mm256_packus_epi32(r3, r3);
            __m256i r5 = _mm256_packus_epi16(r4, r4);
            *(int*)(dst1 + 0) = r5.m256i_u32[0];
            *(int*)(dst1 + 4) = r5.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_V5(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned int SrcMask2[5000]; // for every pixel in source

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned int)(x - PrevBlockEnd) | 0x80808000;
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)(y_diff));
        __m256i yrfv = _mm256_set1_epi32((int)(y_diff_flipped));

        __m256i xv = _mm256_setr_epi32((int)(0 * x_ratio), (int)(1 * x_ratio), (int)(2 * x_ratio), (int)(3 * x_ratio), (int)(4 * x_ratio), (int)(5 * x_ratio), (int)(6 * x_ratio), (int)(7 * x_ratio));

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        unsigned int* SrcMask = SrcMask2;
        //        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m256i xm2 = _mm256_load_si256((const __m256i*)(SrcMask));
            SrcMask += 8;
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00, srcPixels_00);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01, srcPixels_01);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_10, srcPixels_10);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11, srcPixels_11);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels, we are loading 256 bits here
            __m256i lowmask = _mm256_set1_epi32(0xFF);
            __m256i xrv = _mm256_and_si256(xv, lowmask);
            __m256i flipper = _mm256_set1_epi32(256);
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);
            __m256i addx = _mm256_set1_epi32(x_ratio);
            xv = _mm256_add_epi32(xv, addx);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes
            __m256i r3 = _mm256_srli_epi32(r2, 16);
            __m256i r4 = _mm256_packus_epi32(r3, r3);
            __m256i r5 = _mm256_packus_epi16(r4, r4);
            *(int*)(dst1 + 0) = r5.m256i_u32[0];
            *(int*)(dst1 + 4) = r5.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_V4(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    int x_ratio8 = x_ratio * 8;
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)(y_diff));
        __m256i yrfv = _mm256_set1_epi32((int)(y_diff_flipped));

        __m256i xv = _mm256_setr_epi32((int)(0 * x_ratio), (int)(1 * x_ratio), (int)(2 * x_ratio), (int)(3 * x_ratio), (int)(4 * x_ratio), (int)(5 * x_ratio), (int)(6 * x_ratio), (int)(7 * x_ratio));

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t xscaled = (x_ratio * j);
            size_t x = xscaled >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            __m256i subx = _mm256_set1_epi32((int)xscaled);
            __m256i xm0 = _mm256_sub_epi32(xv, subx);
            __m256i xm1 = _mm256_srli_epi32(xm0, 8);
            __m256i NoneMask = _mm256_setr_epi8(00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80, 00, 0x80, 0x80, 0x80);
            __m256i xm2 = _mm256_or_si256(xm1, NoneMask);
            __m256i srcPixels32_00 = _mm256_setr_m128i(srcPixels_00, srcPixels_00);
            srcPixels32_00 = _mm256_shuffle_epi8(srcPixels32_00, xm2);
            __m256i srcPixels32_01 = _mm256_setr_m128i(srcPixels_01, srcPixels_01);
            srcPixels32_01 = _mm256_shuffle_epi8(srcPixels32_01, xm2);
            __m256i srcPixels32_10 = _mm256_setr_m128i(srcPixels_10, srcPixels_10);
            srcPixels32_10 = _mm256_shuffle_epi8(srcPixels32_10, xm2);
            __m256i srcPixels32_11 = _mm256_setr_m128i(srcPixels_11, srcPixels_11);
            srcPixels32_11 = _mm256_shuffle_epi8(srcPixels32_11, xm2);

            // Load xr and xrf for the 8 src pixels, we are loading 256 bits here
            __m256i lowmask = _mm256_set1_epi32(0xFF);
            __m256i xrv = _mm256_and_si256(xv, lowmask);
            __m256i flipper = _mm256_set1_epi32(256);
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);
            __m256i addx = _mm256_set1_epi32(x_ratio8);
            xv = _mm256_add_epi32(xv, addx);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes
            __m256i r3 = _mm256_srli_epi32(r2, 16);
            __m256i r4 = _mm256_packus_epi32(r3, r3);
            __m256i r5 = _mm256_packus_epi16(r4, r4);
            *(int*)(dst1 + 0) = r5.m256i_u32[0];
            *(int*)(dst1 + 4) = r5.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_V3(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned char SrcMask2[5000]; // for every pixel in source

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned char)(x - PrevBlockEnd);
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)(y_diff));
        __m256i yrfv = _mm256_set1_epi32((int)(y_diff_flipped));

        __m256i xv = _mm256_setr_epi32((int)(0* x_ratio), (int)(1 * x_ratio), (int)(2 * x_ratio), (int)(3 * x_ratio), (int)(4 * x_ratio), (int)(5 * x_ratio), (int)(6 * x_ratio), (int)(7 * x_ratio));

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        char* SrcMask = SrcMask2;
//        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m128i srcMaskv = _mm_loadu_si64((const __m128i*)(SrcMask));
            SrcMask += 8;
            srcPixels_00 = _mm_shuffle_epi8(srcPixels_00, srcMaskv);
            srcPixels_01 = _mm_shuffle_epi8(srcPixels_01, srcMaskv);
            srcPixels_10 = _mm_shuffle_epi8(srcPixels_10, srcMaskv);
            srcPixels_11 = _mm_shuffle_epi8(srcPixels_11, srcMaskv);

            // Create 8 integers
            __m256i srcPixels32_00 = _mm256_cvtepu8_epi32(srcPixels_00);
            __m256i srcPixels32_01 = _mm256_cvtepu8_epi32(srcPixels_01);
            __m256i srcPixels32_10 = _mm256_cvtepu8_epi32(srcPixels_10);
            __m256i srcPixels32_11 = _mm256_cvtepu8_epi32(srcPixels_11);

            // Load xr and xrf for the 8 src pixels, we are loading 256 bits here
            __m256i lowmask = _mm256_set1_epi32(0xFF);
            __m256i xrv = _mm256_and_si256(xv, lowmask);
            __m256i flipper = _mm256_set1_epi32(256);
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);
            __m256i addx = _mm256_set1_epi32(x_ratio);
            xv = _mm256_add_epi32(xv, addx);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes
            __m256i r3 = _mm256_srli_epi32(r2, 16);
            __m256i r4 = _mm256_packus_epi32(r3, r3);
            __m256i r5 = _mm256_packus_epi16(r4, r4);
            *(int*)(dst1 + 0) = r5.m256i_u32[0];
            *(int*)(dst1 + 4) = r5.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_V2(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned char SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned char CoeffXr[5000];

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (unsigned char)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned char)(x - PrevBlockEnd);
    }

    __m256i flipper = _mm256_setr_epi32(256, 256, 256, 256, 256, 256, 256, 256);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)y_diff);
        __m256i yrfv = _mm256_set1_epi32((int)y_diff_flipped);

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        char* SrcMask = SrcMask2;
        unsigned char* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m128i srcMaskv = _mm_loadu_si64((const __m128i*)(SrcMask));
            SrcMask += 8;
            srcPixels_00 = _mm_shuffle_epi8(srcPixels_00, srcMaskv);
            srcPixels_01 = _mm_shuffle_epi8(srcPixels_01, srcMaskv);
            srcPixels_10 = _mm_shuffle_epi8(srcPixels_10, srcMaskv);
            srcPixels_11 = _mm_shuffle_epi8(srcPixels_11, srcMaskv);

            // Create 8 integers
            __m256i srcPixels32_00 = _mm256_cvtepu8_epi32(srcPixels_00);
            __m256i srcPixels32_01 = _mm256_cvtepu8_epi32(srcPixels_01);
            __m256i srcPixels32_10 = _mm256_cvtepu8_epi32(srcPixels_10);
            __m256i srcPixels32_11 = _mm256_cvtepu8_epi32(srcPixels_11);

            // Load xr and xrf for the 8 src pixels
            __m128i xrvc = _mm_loadu_si64((const __m256i*)(CoeffXr1));
            __m256i xrv = _mm256_cvtepu8_epi32(xrvc);
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes
            __m256i r3 = _mm256_srli_epi32(r2, 16);
            __m256i r4 = _mm256_packus_epi32(r3, r3);
            __m256i r5 = _mm256_packus_epi16(r4, r4);
            *(int*)(dst1 + 0) = r5.m256i_u32[0];
            *(int*)(dst1 + 4) = r5.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_V1(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned char SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned int CoeffXr[5000];

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (unsigned int)(x_r);
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned char)(x - PrevBlockEnd);
    }

    __m256i flipper = _mm256_setr_epi32(256, 256, 256, 256, 256, 256, 256, 256);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1); // 8 bit value
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff); // 8 bit value

        __m256i yrv = _mm256_set1_epi32((int)(y_diff));
        __m256i yrfv = _mm256_set1_epi32((int)(y_diff_flipped));

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        char* SrcMask = SrcMask2;
        unsigned int *CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcPixels_10 = _mm_loadu_si128((const __m128i*)(&src1[x + srcWidth1]));

             // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);
            __m128i srcPixels_11 = _mm_srli_si128(srcPixels_10, 1);

            // Load src mask(need 8 bytes) and apply it
            __m128i srcMaskv = _mm_loadu_si64((const __m128i*)(SrcMask));
            SrcMask += 8;
            srcPixels_00 = _mm_shuffle_epi8(srcPixels_00, srcMaskv);
            srcPixels_01 = _mm_shuffle_epi8(srcPixels_01, srcMaskv);
            srcPixels_10 = _mm_shuffle_epi8(srcPixels_10, srcMaskv);
            srcPixels_11 = _mm_shuffle_epi8(srcPixels_11, srcMaskv);

            // Create 8 integers
            __m256i srcPixels32_00 = _mm256_cvtepu8_epi32(srcPixels_00);
            __m256i srcPixels32_01 = _mm256_cvtepu8_epi32(srcPixels_01);
            __m256i srcPixels32_10 = _mm256_cvtepu8_epi32(srcPixels_10);
            __m256i srcPixels32_11 = _mm256_cvtepu8_epi32(srcPixels_11);

            // Load xr and xrf for the 8 src pixels, we are loading 256 bits here
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply xr and yr to get coefficients
            // these are all 8 bit values and the result should fit into 16 bit values
            __m256i c00 = _mm256_mullo_epi32(xrfv, yrfv);
            __m256i c01 = _mm256_mullo_epi32(xrv, yrfv);
            __m256i c10 = _mm256_mullo_epi32(xrfv, yrv);
            __m256i c11 = _mm256_mullo_epi32(xrv, yrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, c00);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, c01);
            __m256i pr10 = _mm256_mullo_epi32(srcPixels32_10, c10);
            __m256i pr11 = _mm256_mullo_epi32(srcPixels32_11, c11);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);
            __m256i r1 = _mm256_add_epi32(pr10, pr11);
            __m256i r2 = _mm256_add_epi32(r0, r1);

            // only store bytes from valid indexes
            __m256i r3 = _mm256_srli_epi32(r2, 16);
            __m256i r4 = _mm256_packus_epi32(r3, r3);
            __m256i r5 = _mm256_packus_epi16(r4, r4);
            *(int*)(dst1 + 0) = r5.m256i_u32[0];
            *(int*)(dst1 + 4) = r5.m256i_u32[4];
            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_HI(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    __declspec(align(32)) unsigned char SrcMask2[5000]; // for every pixel in source
    __declspec(align(32)) unsigned int CoeffXr[5000];

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_r = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);

        if (j % 8 == 0)
        {
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block
        }

        CoeffXr[j] = (int)x_r;
        // for every output, we select 1 input out of the 128 loaded
        SrcMask2[j] = (unsigned char)(x - PrevBlockEnd);
    }

    __m256i flipper = _mm256_setr_epi32(256, 256, 256, 256, 256, 256, 256, 256);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value

        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        char* SrcMask = SrcMask2;
        unsigned int* CoeffXr1 = CoeffXr;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

            // Load 128 pixels from src, we will select only a few of the input pixels
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            // We need the neighbours to the pixels so we can interpolate
            __m128i srcPixels_01 = _mm_srli_si128(srcPixels_00, 1);

            // Load src mask(need 8 bytes) and apply it
            __m128i srcMaskv = _mm_loadu_si64((const __m128i*)(SrcMask));
            SrcMask += 8;
            srcPixels_00 = _mm_shuffle_epi8(srcPixels_00, srcMaskv);
            srcPixels_01 = _mm_shuffle_epi8(srcPixels_01, srcMaskv);

            // Create 8 integers
            __m256i srcPixels32_00 = _mm256_cvtepu8_epi32(srcPixels_00);
            __m256i srcPixels32_01 = _mm256_cvtepu8_epi32(srcPixels_01);

            // Load xr and xrf for the 8 src pixels, we are loading 256 bits here
            __m256i xrv = _mm256_load_si256((const __m256i*)(CoeffXr1));
            CoeffXr1 += 8;
            __m256i xrfv = _mm256_sub_epi32(flipper, xrv);

            // multiply coefficients with src
            __m256i pr00 = _mm256_mullo_epi32(srcPixels32_00, xrfv);
            __m256i pr01 = _mm256_mullo_epi32(srcPixels32_01, xrv);

            // add multiple values together
            __m256i r0 = _mm256_add_epi32(pr00, pr01);

            // only store bytes from valid indexes
            {
                __m256i r3 = _mm256_srli_epi32(r0, 16);
                __m256i r4 = _mm256_packus_epi32(r3, r3);
                __m256i r5 = _mm256_packus_epi16(r4, r4);
                *(int*)(dst1 + 0) = r5.m256i_u32[0];
                *(int*)(dst1 + 4) = r5.m256i_u32[4];
            }

//            * (__int64*)(dst1 + 0) = r0.m256i_u32[0];

            dst1 += 8;
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_Resize_Simple(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    unsigned char SrcMask2[5000]; // for every pixel in source

    assert(dstWidth < sizeof(SrcMask2));

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

        if (j % 8 == 0)
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block

        SrcMask2[j] = (unsigned char)(x - PrevBlockEnd);
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[(y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        size_t xsum = 0;
        size_t x_ratio2 = x_ratio * 8;
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = xsum >> FLOAT_PRECISSION_BITS2;
            xsum += x_ratio2;
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[x]));
            __m128i srcMaskv = _mm_loadu_si64((const __m128i*)(&SrcMask2[j]));
            __m128i res = _mm_shuffle_epi8(srcPixels_00, srcMaskv);
            _mm_storeu_si64(&dst1[j], res);
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_Read(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        __m128i sum = _mm_set1_epi8(0);
        for (int j = 0; j < dstWidth; j += 8)
        {
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[j]));
            sum = _mm_adds_epi8(sum, srcPixels_00);
        }
        _mm_storeu_epi64(&dst1[0], sum);
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_Write(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src1[0]));
        for (int j = 0; j < dstWidth; j += 8)
        {
            _mm_storeu_epi64(&dst1[j], srcPixels_00);
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_JustCopy(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        for (int j = 0; j < dstWidth; j += 8)
        {
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*) &src1[j]);
            _mm_storeu_epi64(&dst1[j], srcPixels_00);
        }
/*
* this function gets compiled into
00007FF70BE91C52  lea         rdx,[rdx+8]
00007FF70BE91C56  movdqu      xmm0,xmmword ptr [rax]
00007FF70BE91C5A  lea         rax,[rax+8]
00007FF70BE91C5E  vmovdqu     xmmword ptr [rdx-8],xmm0
00007FF70BE91C63  sub         r8,1
00007FF70BE91C67  jne         main+672h (07FF70BE91C52h)  
*/
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_CopyAligned(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        src1 = src1 + (32 - (int)(src1) % 32);
        uchar* dst1 = &dst[i * dstWidth];
        dst1 = dst1 + (32 - (int)(dst1) % 32);
        for (int j = 0; j < dstWidth; j += 8)
        {
            __m128i srcPixels_00 = _mm_load_si128((const __m128i*) &src1[j]);
            _mm_storeu_si64(&dst1[j], srcPixels_00);
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_CopyAlignedSrc(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        src1 = src1 + (32 - (int)(src1) % 32);
        uchar* dst1 = &dst[i * dstWidth];
//        dst1 = dst1 + (32 - (int)(dst1) % 32);
        for (int j = 0; j < dstWidth; j += 8)
        {
            __m128i srcPixels_00 = _mm_load_si128((const __m128i*) &src1[j]);
            _mm_storeu_si64(&dst1[j], srcPixels_00);
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_Copy2(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        uchar* dst1 = &dst[i * dstWidth];
        for (int j = 0; j < dstWidth; j += 8)
        {
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*) &src1[j]);
            _mm_storeu_si64(&dst1[j], srcPixels_00);
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_Y_Resize(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    // We presume the relation of loading less pixels from source to destination
    assert(dstWidth <= srcWidth1);

    unsigned char SrcMask2[5000]; // for every pixel in source

    assert(dstWidth < sizeof(SrcMask2));

    size_t PrevBlockEnd = 0;
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;

        if (j % 8 == 0)
            PrevBlockEnd = x; // need to increase src by this amount to be able to load data for next block

        SrcMask2[j] = (unsigned char)(x - PrevBlockEnd);
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2; // 16 bit value
        for (int j = 0; j < dstWidth; j += 8)
        {
            size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
            __m128i srcPixels_00 = _mm_loadu_si128((const __m128i*)(&src[ ((size_t)y + cropY) * srcWidth1 + cropX + x]));
            __m128i srcMaskv = _mm_loadu_si64((const __m128i*)(&SrcMask2[j]));
            srcPixels_00 = _mm_shuffle_epi8(srcPixels_00, srcMaskv);
            _mm_storeu_epi64(&dst[i * dstWidth+j], srcPixels_00);
        }
    }
}

void nv12_bilinear_scale_with_crop_AVX_UV_V1(uchar * src, uchar * dst,
        int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
    {

    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;
    
    __declspec(align(32)) size_t CoeffMapX[12000];
    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_diff = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        CoeffMapX[j * 3 + 0] = x;
        CoeffMapX[j * 3 + 1] = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
        CoeffMapX[j * 3 + 2] = x_diff;
    }

    int srcuvWidth1 = srcWidth1 / 2;
    //    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    uchar* dstuv = &dst[dstoffset];
    for (size_t i = 0; i < dstHeight / 2; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[(size_t)y * srcWidth1 + srcoffset];
        uchar* dstuv1 = &dstuv[i * dstWidth];
        for (size_t j = 0; j < dstWidth / 2; j++)
        {
            size_t x = CoeffMapX[j * 3 + 0];
            x = x * 2;
            size_t x_diff = CoeffMapX[j * 3 + 1];
            size_t x_diff_flipped = CoeffMapX[j * 3 + 2];
            size_t k[4];
            k[0] = x_diff_flipped * y_diff_flipped;
            k[1] = x_diff * y_diff_flipped;
            k[2] = y_diff * x_diff_flipped;
            k[3] = x_diff * y_diff;
            {
                size_t vals[2][2];
                vals[0][0] = src1[x];
                vals[0][1] = src1[x + 2];
                vals[1][0] = src1[x + srcWidth1];
                vals[1][1] = src1[x + srcWidth1 + 2];
                size_t res = vals[0][0] * k[0]
                    + vals[0][1] * k[1]
                    + vals[1][0] * k[2]
                    + vals[0][1] * k[3];
                res = (res >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res <= 255);
                dstuv1[j * 2] = (uchar)res;
            }

            {
                size_t vals2[2][2];
                vals2[0][0] = src1[x + 1];
                vals2[0][1] = src1[x + 3];
                vals2[1][0] = src1[x + srcWidth1 + 1];
                vals2[1][1] = src1[x + srcWidth1 + 3];
                size_t res2 = vals2[0][0] * k[0]
                    + vals2[0][1] * k[1]
                    + vals2[1][0] * k[2]
                    + vals2[0][1] * k[3];
                res2 = (res2 >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res2 <= 255);
                dstuv1[j * 2 + 1] = (uchar)res2;
            }
        }
    }
}

void nv12_direct_copy(uchar* src, uchar* dst, int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{

    int srcWidth = cropWidth, srcHeight = cropHeight;

    assert(cropWidth == dstWidth);

    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        memcpy(&dst[i * dstWidth], &src[((size_t)y + cropY) * srcWidth1 + cropX], dstWidth);
    }

    uchar* dstuv = &dst[dstoffset];
    for (size_t i = 0; i < dstHeight / 2; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        memcpy(&dstuv[i * dstWidth], &src[(size_t)y * srcWidth1 + srcoffset], dstWidth);
    }
}

void nv12_bilinear_scale_with_crop_AVX_V1(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    /*
v1
Time elapsed with crop before resize, 197475
Time elapsed for built-in crop,       94130
Time elapsed with crop before resize, 194509
Time elapsed for built-in crop,       96429

v2
Time elapsed with crop before resize, 199973
Time elapsed for built-in crop,       91255
Time elapsed with crop before resize, 205144
Time elapsed for built-in crop,       90100

v3
Time elapsed with crop before resize, 186858
Time elapsed for built-in crop,       104959
Time elapsed with crop before resize, 208816
Time elapsed for built-in crop,       98739

v4
Time elapsed with crop before resize, 184921
Time elapsed for built-in crop,       116555
Time elapsed with crop before resize, 186515
Time elapsed for built-in crop,       115055

v5
Time elapsed with crop before resize, 184844
Time elapsed for built-in crop,       100335
Time elapsed with crop before resize, 183468
Time elapsed for built-in crop,       102070

v6
Time elapsed with crop before resize, 205113
Time elapsed for built-in crop,       88944
Time elapsed with crop before resize, 198796
Time elapsed for built-in crop,       85726
*/
    if (dstWidth == cropWidth)
        nv12_direct_copy(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    else 
        if ((float)cropWidth / (float)dstWidth <= (float)15 / (float)8 && (float)cropWidth / (float)dstWidth >= 1)
            //nv12_bilinear_scale_with_crop_AVX_YUV_V1(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
            nv12_bilinear_scale_with_crop_AVX_YUV_V2(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    else
        nv12_bilinear_scale_with_crop_VI3I(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V7(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V6(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V5(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V4(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V3(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V2(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_V1(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_HI(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_Resize_Simple(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    //56819
//    nv12_bilinear_scale_with_crop_AVX_Y_Resize(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
//    nv12_bilinear_scale_with_crop_AVX_Y_Resize2(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    // 52000
//    nv12_bilinear_scale_with_crop_AVX_Y_JustCopy(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    // 6400
//    nv12_bilinear_scale_with_crop_AVX_Y_CopyAligned(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    // 7327
//    nv12_bilinear_scale_with_crop_AVX_Y_CopyAlignedSrc(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    // 6421
//    nv12_bilinear_scale_with_crop_AVX_Y_Copy2(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    // 2400
//    nv12_bilinear_scale_with_crop_AVX_Y_Read(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
    // 3400
//    nv12_bilinear_scale_with_crop_AVX_Y_Write(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);

#ifndef DISABLE_UV_GEN
//    nv12_bilinear_scale_with_crop_AVX_UV_V1(src, dst, srcWidth1, srcHeight1, dstWidth, dstHeight, cropX, cropY, cropWidth, cropHeight);
#endif
}