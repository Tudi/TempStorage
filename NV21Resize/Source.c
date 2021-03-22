#include <intrin.h>
#include "Source.h"
#include <assert.h>

#define FLOAT_PRECISSION_BITS 16
#define FLOAT_PRECISSION_BITS2 8

void nv12_bilinear_scale_with_crop_VI1(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    float x_ratio = ((float)(srcWidth - 1)) / dstWidth;
    float y_ratio = ((float)(srcHeight - 1)) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    float CoeffMapX[8000];

    for (int j = 0; j < dstWidth; j++)
    {
        int x = (int)(x_ratio * j);
        float x_diff = (x_ratio * j) - x;
        *(int*)&CoeffMapX[j * 3 + 0] = x;
        CoeffMapX[j * 3 + 1] = (1 - x_diff);
        CoeffMapX[j * 3 + 2] = x_diff;
    }

    for (int i = 0; i < dstHeight; i++) 
    {
        float y = y_ratio * i;
        float y_diff = y - (int)y;
        float y_diff_flipped = 1.0f - y_diff;
        uchar* src1 = &src[((int)y + cropY) * srcWidth1 + cropX];
        for (int j = 0; j < dstWidth; j++)
        {
            int x = *(int*)&CoeffMapX[j*3 + 0];
            float x_diff = CoeffMapX[j * 3 + 1];
            float x_diff_flipped = CoeffMapX[j * 3 + 2];
            dst[i * dstWidth + j] = (int)(src1[x] * x_diff_flipped * y_diff_flipped
                + src1[x + 1] * (x_diff) * y_diff_flipped
                + src1[x + srcWidth1] * (y_diff) * x_diff_flipped
                + src1[x + srcWidth1 + 1] * (x_diff * y_diff));
        }
    }

    for (int i = 0; i < dstHeight / 2; i++) 
    {
        float y = (y_ratio * i);
        float y_diff = y - (int)y;
        float y_diff_flipped = 1.0f - y_diff;
        uchar* src1 = &src[(int)y * srcWidth1 + srcoffset];
        for (int j = 0; j < dstWidth / 2; j++)
        {
            int x = *(int*)&CoeffMapX[j * 3 + 0];
            x *= 2;
            float x_diff = CoeffMapX[j * 3 + 1];
            float x_diff_flipped = CoeffMapX[j * 3 + 2];
            dst[dstoffset + 2 * (i * dstWidth / 2 + j)] = (int)(
                src1[x] * x_diff_flipped * y_diff_flipped 
                + src1[x + 2] * (x_diff) * y_diff_flipped
                + src1[x + srcWidth1] * (y_diff) * x_diff_flipped
                + src1[x + srcWidth1+2] * (x_diff * y_diff));
            dst[dstoffset + 2 * (i * dstWidth / 2 + j) + 1] = (int)(
                src1[x + 1] * x_diff_flipped * y_diff_flipped
                + src1[x + 3] * (x_diff) * y_diff_flipped 
                + src1[x + srcWidth1 + 1] * (y_diff) * x_diff_flipped
                + src1[x + srcWidth1 + 3] * (x_diff * y_diff));
        }
    }
}

void nv12_bilinear_scale_with_crop_VI1I(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
//    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    __declspec(align(16)) size_t CoeffMapX[8000];

    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_diff = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2)-1);
        CoeffMapX[j * 3 + 0] = x;
        CoeffMapX[j * 3 + 1] = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
        CoeffMapX[j * 3 + 2] = x_diff;
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1<<FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        for (int j = 0; j < dstWidth; j++)
        {
            size_t x = CoeffMapX[j * 3 + 0];
            size_t x_diff = CoeffMapX[j * 3 + 1];
            size_t x_diff_flipped = CoeffMapX[j * 3 + 2];
            size_t vals[2][2];
            vals[0][0] = src1[x];
            vals[0][1] = src1[x + 1];
            vals[1][0] = src1[x + srcWidth1];
            vals[1][1] = src1[x + srcWidth1 + 1];
            size_t res = vals[0][0] * x_diff_flipped * y_diff_flipped
                + vals[0][1] * x_diff * y_diff_flipped
                + vals[1][0] * y_diff * x_diff_flipped
                + vals[0][1] * x_diff * y_diff;
            res = (res >> (2 * FLOAT_PRECISSION_BITS2));
            assert(res <= 255);
            dst[i * dstWidth + j] = (uchar)res;
        }
    }
    
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
            {
                size_t vals[2][2];
                vals[0][0] = src1[x];
                vals[0][1] = src1[x + 2];
                vals[1][0] = src1[x + srcWidth1];
                vals[1][1] = src1[x + srcWidth1 + 2];
                size_t res = vals[0][0] * x_diff_flipped * y_diff_flipped
                    + vals[0][1] * x_diff * y_diff_flipped
                    + vals[1][0] * y_diff * x_diff_flipped
                    + vals[0][1] * x_diff * y_diff;
                res = (res >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res <= 255);
                dstuv1[j*2] = (uchar)res;
            }

            {
                size_t vals2[2][2];
                vals2[0][0] = src1[x + 1];
                vals2[0][1] = src1[x + 3];
                vals2[1][0] = src1[x + srcWidth1 + 1];
                vals2[1][1] = src1[x + srcWidth1 + 3];
                size_t res2 = vals2[0][0] * x_diff_flipped * y_diff_flipped
                    + vals2[0][1] * x_diff * y_diff_flipped
                    + vals2[1][0] * y_diff * x_diff_flipped
                    + vals2[0][1] * x_diff * y_diff;
                res2 = (res2 >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res2 <= 255);
                dstuv1[j * 2 + 1] = (uchar)res2;
            }
        }
    }
}

void nv12_bilinear_scale_with_crop_VI2I(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
    //    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    __declspec(align(16)) size_t CoeffMapX[8000];

    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_diff = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        CoeffMapX[j * 2 + 0] = x;
        CoeffMapX[j * 2 + 1] = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        for (int j = 0; j < dstWidth; j++)
        {
            size_t x = CoeffMapX[j * 2 + 0];
            size_t x_diff = CoeffMapX[j * 2 + 1];
            size_t x_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
            size_t vals[2][2];
            vals[0][0] = src1[x];
            vals[0][1] = src1[x + 1];
            vals[1][0] = src1[x + srcWidth1];
            vals[1][1] = src1[x + srcWidth1 + 1];
            size_t res = vals[0][0] * x_diff_flipped * y_diff_flipped
                + vals[0][1] * x_diff * y_diff_flipped
                + vals[1][0] * y_diff * x_diff_flipped
                + vals[0][1] * x_diff * y_diff;
            res = (res >> (2 * FLOAT_PRECISSION_BITS2));
            assert(res <= 255);
            dst[i * dstWidth + j] = (uchar)res;
        }
    }

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
            size_t x = CoeffMapX[j * 2 + 0];
            x = x * 2;
            size_t x_diff = CoeffMapX[j * 2 + 1];
            size_t x_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
            {
                size_t vals[2][2];
                vals[0][0] = src1[x];
                vals[0][1] = src1[x + 2];
                vals[1][0] = src1[x + srcWidth1];
                vals[1][1] = src1[x + srcWidth1 + 2];
                size_t res = vals[0][0] * x_diff_flipped * y_diff_flipped
                    + vals[0][1] * x_diff * y_diff_flipped
                    + vals[1][0] * y_diff * x_diff_flipped
                    + vals[0][1] * x_diff * y_diff;
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
                size_t res2 = vals2[0][0] * x_diff_flipped * y_diff_flipped
                    + vals2[0][1] * x_diff * y_diff_flipped
                    + vals2[1][0] * y_diff * x_diff_flipped
                    + vals2[0][1] * x_diff * y_diff;
                res2 = (res2 >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res2 <= 255);
                dstuv1[j * 2 + 1] = (uchar)res2;
            }
        }
    }
}


void nv12_bilinear_scale_with_crop_VI3I(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
    //    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    __declspec(align(16)) size_t CoeffMapX[8000];

    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_diff = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        CoeffMapX[j * 3 + 0] = x;
        CoeffMapX[j * 3 + 1] = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
        CoeffMapX[j * 3 + 2] = x_diff;
    }

    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        for (int j = 0; j < dstWidth; j++)
        {
            size_t x = CoeffMapX[j * 3 + 0];
            size_t x_diff = CoeffMapX[j * 3 + 1];
            size_t x_diff_flipped = CoeffMapX[j * 3 + 2];
            size_t vals[2][2];
            vals[0][0] = src1[x];
            vals[0][1] = src1[x + 1];
            vals[1][0] = src1[x + srcWidth1];
            vals[1][1] = src1[x + srcWidth1 + 1];
            size_t k[4];
            k[0] = x_diff_flipped * y_diff_flipped;
            k[1] = x_diff * y_diff_flipped;
            k[2] = y_diff * x_diff_flipped;
            k[3] = x_diff * y_diff;
            size_t res = vals[0][0] * k[0]
                + vals[0][1] * k[1]
                + vals[1][0] * k[2]
                + vals[0][1] * k[3];
            res = (res >> (2 * FLOAT_PRECISSION_BITS2));
            assert(res <= 255);
            dst[i * dstWidth + j] = (uchar)res;
        }
    }

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

void nv12_bilinear_scale_with_crop_VI4I(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
#ifndef FLOAT_PRECISSION_BITS2
#define FLOAT_PRECISSION_BITS2 8 // on x64 systems, this could be increased to up to 24 bits
#endif

    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
    //    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    __declspec(align(16)) size_t CoeffMapX[8000];

    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_diff = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        CoeffMapX[j * 3 + 0] = x;
        CoeffMapX[j * 3 + 1] = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
        CoeffMapX[j * 3 + 2] = x_diff;
    }

    assert((dstWidth % 4) == 0);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        for (int j = 0; j < dstWidth; j += 4)
        {
            size_t res = 0;
            for (int j2 = 0; j2 < 4; j2++)
            {
                size_t x = CoeffMapX[(j2 + j) * 3 + 0];
                size_t x_diff = CoeffMapX[(j2 + j) * 3 + 1];
                size_t x_diff_flipped = CoeffMapX[(j2 + j) * 3 + 2];
                size_t vals[2][2];
                vals[0][0] = src1[x];
                vals[0][1] = src1[x + 1];
                vals[1][0] = src1[x + srcWidth1];
                vals[1][1] = src1[x + srcWidth1 + 1];
                size_t k[4];
                k[0] = x_diff_flipped * y_diff_flipped;
                k[1] = x_diff * y_diff_flipped;
                k[2] = y_diff * x_diff_flipped;
                k[3] = x_diff * y_diff;
                size_t res_t = vals[0][0] * k[0]
                    + vals[0][1] * k[1]
                    + vals[1][0] * k[2]
                    + vals[0][1] * k[3];
                res_t = (res_t >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res_t <= 255);
                res |= (res_t << (j2 * 8));
            }
            *(unsigned int*)&dst[i * dstWidth + j] = (unsigned int)res;
        }
    }

    uchar* dstuv = &dst[dstoffset];
    for (size_t i = 0; i < dstHeight / 2; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[(size_t)y * srcWidth1 + srcoffset];
        uchar* dstuv1 = &dstuv[i * dstWidth];
        for (size_t j = 0; j < dstWidth / 2; j += 2)
        {
            size_t res_uv = 0;
            for (int j2 = 0; j2 < 2; j2++)
            {
                size_t x = CoeffMapX[(j + j2) * 3 + 0];
                x = x * 2;
                size_t x_diff = CoeffMapX[(j + j2) * 3 + 1];
                size_t x_diff_flipped = CoeffMapX[(j + j2) * 3 + 2];
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
                    res_uv |= (res << (j2 * 2 * 8));
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
                    res_uv |= (res2 << ((j2 * 2 + 1) * 8));
                }
            }
            *(unsigned int*)&dstuv1[j * 2] = (unsigned int)res_uv;
        }
    }
}


void nv12_bilinear_scale_with_crop_VI5I(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
#ifndef FLOAT_PRECISSION_BITS2
#define FLOAT_PRECISSION_BITS2 8 // on x64 systems, this could be increased to up to 24 bits
#endif

    int srcWidth = cropWidth, srcHeight = cropHeight;
    int x_ratio = ((srcWidth - 1) << FLOAT_PRECISSION_BITS2) / dstWidth;
    int y_ratio = ((srcHeight - 1) << FLOAT_PRECISSION_BITS2) / dstHeight;

    int srcuvWidth1 = srcWidth1 / 2;
    //    int srcuvHeight1 = srcHeight1 / 2;
    int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
    int dstoffset = dstHeight * dstWidth;

    __declspec(align(16)) size_t CoeffMapX[8000];

    for (int j = 0; j < dstWidth; j++)
    {
        size_t x = (x_ratio * j) >> FLOAT_PRECISSION_BITS2;
        size_t x_diff = (x_ratio * j) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        CoeffMapX[j * 3 + 0] = x;
        CoeffMapX[j * 3 + 1] = ((1 << FLOAT_PRECISSION_BITS2) - x_diff);
        CoeffMapX[j * 3 + 2] = x_diff;
    }
    // when reading out of bounds, make sure we index a valid location
    for (int j = dstWidth; j < dstWidth + 16; j++)
    {
        CoeffMapX[j * 3 + 0] = 0;
        CoeffMapX[j * 3 + 1] = 0;
        CoeffMapX[j * 3 + 2] = 0;
    }

    assert((dstWidth % 4) == 0);
    for (size_t i = 0; i < dstHeight; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[((size_t)y + cropY) * srcWidth1 + cropX];
        for (int j = 0; j < dstWidth; j += 4)
        {
            __declspec(align(16)) size_t row1[8];
            __declspec(align(16)) size_t row2[8];
            for (int j2 = 0; j2 < 4; j2++)
            {
                size_t x = CoeffMapX[(j2 + j) * 3 + 0];
                row1[j2 * 2 + 0] = src1[x + 0];
                row1[j2 * 2 + 1] = src1[x + 1];
            }
            for (int j2 = 0; j2 < 4; j2++)
            {
                size_t x = CoeffMapX[(j2 + j) * 3 + 0];
                row2[j2 * 2 + 0] = src1[x + srcWidth1 + 0];
                row2[j2 * 2 + 1] = src1[x + srcWidth1 + 1];
            }
            size_t res = 0;
            for (int j2 = 0; j2 < 4; j2++)
            {
                //				size_t x = CoeffMapX[(j2+j) * 3 + 0];
                size_t x_diff = CoeffMapX[(j2 + j) * 3 + 1];
                size_t x_diff_flipped = CoeffMapX[(j2 + j) * 3 + 2];
                size_t k[4];
                k[0] = x_diff_flipped * y_diff_flipped;
                k[1] = x_diff * y_diff_flipped;
                k[2] = y_diff * x_diff_flipped;
                k[3] = x_diff * y_diff;
                size_t res_t = row1[j2 * 2 + 0] * k[0]
                    + row1[j2 * 2 + 1] * k[1]
                    + row2[j2 * 2 + 0] * k[2]
                    + row2[j2 * 2 + 1] * k[3];
                res_t = (res_t >> (2 * FLOAT_PRECISSION_BITS2));
                assert(res_t <= 255);
                res |= (res_t << (j2 * 8));
            }
            //			*(unsigned int*)&dst[i * dstWidth + j] = (unsigned int)res;
            *(unsigned long long*)& dst[i * dstWidth + j] = (unsigned long long)res;
        }
    }

    uchar* dstuv = &dst[dstoffset];
    for (size_t i = 0; i < dstHeight / 2; i++)
    {
        size_t y = (y_ratio * i) >> FLOAT_PRECISSION_BITS2;
        size_t y_diff = (y_ratio * i) & ((1 << FLOAT_PRECISSION_BITS2) - 1);
        size_t y_diff_flipped = ((1 << FLOAT_PRECISSION_BITS2) - y_diff);
        uchar* src1 = &src[(size_t)y * srcWidth1 + srcoffset];
        uchar* dstuv1 = &dstuv[i * dstWidth];
        for (size_t j = 0; j < dstWidth / 2; j += 2)
        {
            size_t res_uv = 0;
            for (int j2 = 0; j2 < 2; j2++)
            {
                size_t x = CoeffMapX[(j + j2) * 3 + 0];
                x = x * 2;
                size_t x_diff = CoeffMapX[(j + j2) * 3 + 1];
                size_t x_diff_flipped = CoeffMapX[(j + j2) * 3 + 2];
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
                    res_uv |= (res << (j2 * 2 * 8));
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
                    res_uv |= (res2 << ((j2 * 2 + 1) * 8));
                }
            }
            *(unsigned int*)&dstuv1[j * 2] = (unsigned int)res_uv;
        }
    }
}

void DownSample1PlaneLiniarSSE3(unsigned char* src, unsigned char* dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride, unsigned char* RowMask16, unsigned char* Segments)
{
    int int_conv_y = ((SrcH << FLOAT_PRECISSION_BITS) / DestH);
    unsigned int stacking_precission_y = 0;
    int PrevSrcRow = -1;

    unsigned int int_conv_x = (unsigned int)((DestW << FLOAT_PRECISSION_BITS) / SrcW);
    unsigned int int_conv_x_16 = 16 * int_conv_x;
    for (int y = 0; y < DestH; y++)
    {
        int SrcRowNow = stacking_precission_y >> FLOAT_PRECISSION_BITS;
        stacking_precission_y += int_conv_y;
        {
            unsigned char* tdst = dst + y * DestStride;
            unsigned char* tsrc = src + SrcRowNow * SrcStride;
            unsigned char* tseg = Segments; //really need to remake this when i have the time
            //load 16 bytes from src
            for (int x = 0; x < DestW; )
            {
                if (*tseg)
                {
                    __m128i BuffIn = _mm_loadu_si128((__m128i*)(tsrc + 0));
                    tsrc += 16;
                    __m128i CopyMask = _mm_loadu_si128((__m128i*)(RowMask16 + x));
                    __m128i BuffOut = _mm_shuffle_epi8(BuffIn, CopyMask);
                    _mm_storeu_si128((__m128i*)(tdst + x), BuffOut);
                    x += *tseg;
                }
                tseg++;
            }
        }
        PrevSrcRow = SrcRowNow;
    }
}

void nv12_linear_scale_with_crop_VI2(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int ConversionRatioWidth1Pel = ((cropWidth << FLOAT_PRECISSION_BITS) / dstWidth);
    int ConversionRatioWidth16Pel = 16 * ConversionRatioWidth1Pel;
    //index vector : bytes positions : dest from src
    __declspec(align(16)) unsigned char RowMask16[8000];
    __declspec(align(16)) unsigned char Segments[8000]; //this is very lame, but i received a new task and have to close this
    unsigned int MaxMaskUsed = dstWidth + (16 * ConversionRatioWidth1Pel >> FLOAT_PRECISSION_BITS);
#ifdef _DEBUG
    assert(MaxMaskUsed < 8000);
#endif
    if (MaxMaskUsed > 8000)
        return;
    //take 16 bytes from src and generate X output
    int BaseShift = 0;
    int SegmentInd = 0;
    int prevSegmentInd = 0;
    for (signed int i = 0; i < dstWidth; i++)
    {
        int converted_col_index = (i * ConversionRatioWidth1Pel) >> FLOAT_PRECISSION_BITS;
        if (converted_col_index - BaseShift >= 16)
        {
            BaseShift = converted_col_index & ~0x0F;
            Segments[SegmentInd++] = i - prevSegmentInd;
            prevSegmentInd = i;
        }
        int SegmentedIndex = converted_col_index - BaseShift;
        RowMask16[i] = SegmentedIndex;
    }
    Segments[SegmentInd++] = 255;
    uchar* SrcY = src + (16 - ((__int64)src) & 0x0F); // needs to be 16 byte alligned
    uchar* DstY = dst + (16 - ((__int64)dst) & 0x0F); // needs to be 16 byte alligned
    DownSample1PlaneLiniarSSE3(SrcY, DstY, srcWidth1, srcHeight1, srcWidth1, dstWidth, dstHeight, dstWidth, RowMask16, Segments);

    {
        int srcWidth = cropWidth, srcHeight = cropHeight;
        float x_ratio = ((float)(srcWidth - 1)) / dstWidth;
        float y_ratio = ((float)(srcHeight - 1)) / dstHeight;

        int srcuvWidth1 = srcWidth1 / 2;
        int srcuvHeight1 = srcHeight1 / 2;
        int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
        int dstoffset = dstHeight * dstWidth;

        float CoeffMapX[8000];

        for (int j = 0; j < dstWidth; j++)
        {
            int x = (int)(x_ratio * j);
            float x_diff = (x_ratio * j) - x;
            *(int*)&CoeffMapX[j * 3 + 0] = x;
            CoeffMapX[j * 3 + 1] = (1 - x_diff);
            CoeffMapX[j * 3 + 2] = x_diff;
        }

        for (int i = 0; i < dstHeight / 2; i++)
        {
            float y = (y_ratio * i);
            float y_diff = y - (int)y;
            float y_diff_flipped = 1.0f - y_diff;
            uchar* src1 = &src[(int)y * srcWidth1 + srcoffset];
            for (int j = 0; j < dstWidth / 2; j++)
            {
                int x = *(int*)&CoeffMapX[j * 3 + 0];
                x *= 2;
                float x_diff = CoeffMapX[j * 3 + 1];
                float x_diff_flipped = CoeffMapX[j * 3 + 2];
                dst[dstoffset + 2 * (i * dstWidth / 2 + j)] = (int)(
                    src1[x] * x_diff_flipped * y_diff_flipped
                    + src1[x + 2] * (x_diff)*y_diff_flipped
                    + src1[x + srcWidth1] * (y_diff)*x_diff_flipped
                    + src1[x + srcWidth1 + 2] * (x_diff * y_diff));
                dst[dstoffset + 2 * (i * dstWidth / 2 + j) + 1] = (int)(
                    src1[x + 1] * x_diff_flipped * y_diff_flipped
                    + src1[x + 3] * (x_diff)*y_diff_flipped
                    + src1[x + srcWidth1 + 1] * (y_diff)*x_diff_flipped
                    + src1[x + srcWidth1 + 3] * (x_diff * y_diff));
            }
        }

    }
}


void DownSample1PlaneBiLiniar1SSE3(unsigned char* src, unsigned char* dst, int SrcW, int SrcH, int SrcStride, int DestW, int DestH, int DestStride, unsigned char* RowMask16, unsigned char* Segments)
{
    int int_conv_y = ((SrcH << FLOAT_PRECISSION_BITS) / DestH);
    unsigned int stacking_precission_y = 0;
    int PrevSrcRow = -1;

    unsigned int int_conv_x = (unsigned int)((DestW << FLOAT_PRECISSION_BITS) / SrcW);
    unsigned int int_conv_x_16 = 16 * int_conv_x;
    for (int y = 0; y < DestH; y++)
    {
        int SrcRowNow = stacking_precission_y >> FLOAT_PRECISSION_BITS;
        stacking_precission_y += int_conv_y;
        {
            unsigned char* tdst = dst + y * DestStride;
            unsigned char* tsrc = src + SrcRowNow * SrcStride;
            unsigned char* tseg = Segments; //really need to remake this when i have the time
            //load 16 bytes from src
            for (int x = 0; x < DestW; )
            {
                if (*tseg)
                {
                    __m128i BuffIn = _mm_loadu_si128((__m128i*)(tsrc + 0));
                    __m128i BuffIn2 = _mm_slli_si128(BuffIn,8); // we need the values for the interpolation
                    tsrc += 16;
                    __m128i CopyMask = _mm_loadu_si128((__m128i*)(RowMask16 + x));
                    __m128i BuffOut = _mm_shuffle_epi8(BuffIn, CopyMask);
                    __m128i BuffOut2 = _mm_shuffle_epi8(BuffIn2, CopyMask);
                    // based on the coeffs, we should multiply the values
                    // we can only store 4 floats in the register
                    _mm_storeu_si128((__m128i*)(tdst + x), BuffOut);
                    x += *tseg;
                }
                tseg++;
            }
        }
        PrevSrcRow = SrcRowNow;
    }
}

void nv12_bilinear1_scale_with_crop_VI2(uchar* src, uchar* dst,
    int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int ConversionRatioWidth1Pel = ((cropWidth << FLOAT_PRECISSION_BITS) / dstWidth);
    int ConversionRatioWidth16Pel = 16 * ConversionRatioWidth1Pel;
    //index vector : bytes positions : dest from src
    __declspec(align(16)) unsigned char RowMask16[8000];
    __declspec(align(16)) unsigned char Segments[8000]; //this is very lame, but i received a new task and have to close this
    unsigned int MaxMaskUsed = dstWidth + (16 * ConversionRatioWidth1Pel >> FLOAT_PRECISSION_BITS);
#ifdef _DEBUG
    assert(MaxMaskUsed < 8000);
#endif
    if (MaxMaskUsed > 8000)
        return;
    //take 16 bytes from src and generate X output
    int BaseShift = 0;
    int SegmentInd = 0;
    int prevSegmentInd = 0;
    for (signed int i = 0; i < dstWidth; i++)
    {
        int converted_col_index = (i * ConversionRatioWidth1Pel) >> FLOAT_PRECISSION_BITS;
        if (converted_col_index - BaseShift >= 16)
        {
            BaseShift = converted_col_index & ~0x0F;
            Segments[SegmentInd++] = i - prevSegmentInd;
            prevSegmentInd = i;
        }
        int SegmentedIndex = converted_col_index - BaseShift;
        RowMask16[i] = SegmentedIndex;
    }
    Segments[SegmentInd++] = 255;
    uchar* SrcY = src + cropY * srcWidth1 + cropX;
    SrcY = SrcY + (16 - ((__int64)SrcY) & 0x0F); // needs to be 16 byte alligned
    uchar* DstY = dst + (16 - ((__int64)dst) & 0x0F); // needs to be 16 byte alligned
    DownSample1PlaneBiLiniar1SSE3(SrcY, DstY, srcWidth1, srcHeight1, srcWidth1, dstWidth, dstHeight, dstWidth, RowMask16, Segments);

    {
        int srcWidth = cropWidth, srcHeight = cropHeight;
        float x_ratio = ((float)(srcWidth - 1)) / dstWidth;
        float y_ratio = ((float)(srcHeight - 1)) / dstHeight;

        int srcuvWidth1 = srcWidth1 / 2;
        int srcuvHeight1 = srcHeight1 / 2;
        int srcoffset = srcHeight1 * srcWidth1 + 2 * (cropY / 2 * srcuvWidth1 + cropX / 2);
        int dstoffset = dstHeight * dstWidth;

        float CoeffMapX[8000];

        for (int j = 0; j < dstWidth; j++)
        {
            int x = (int)(x_ratio * j);
            float x_diff = (x_ratio * j) - x;
            *(int*)&CoeffMapX[j * 3 + 0] = x;
            CoeffMapX[j * 3 + 1] = (1 - x_diff);
            CoeffMapX[j * 3 + 2] = x_diff;
        }

        for (int i = 0; i < dstHeight / 2; i++)
        {
            float y = (y_ratio * i);
            float y_diff = y - (int)y;
            float y_diff_flipped = 1.0f - y_diff;
            uchar* src1 = &src[(int)y * srcWidth1 + srcoffset];
            for (int j = 0; j < dstWidth / 2; j++)
            {
                int x = *(int*)&CoeffMapX[j * 3 + 0];
                x *= 2;
                float x_diff = CoeffMapX[j * 3 + 1];
                float x_diff_flipped = CoeffMapX[j * 3 + 2];
                dst[dstoffset + 2 * (i * dstWidth / 2 + j)] = (int)(
                    src1[x] * x_diff_flipped * y_diff_flipped
                    + src1[x + 2] * (x_diff)*y_diff_flipped
                    + src1[x + srcWidth1] * (y_diff)*x_diff_flipped
                    + src1[x + srcWidth1 + 2] * (x_diff * y_diff));
                dst[dstoffset + 2 * (i * dstWidth / 2 + j) + 1] = (int)(
                    src1[x + 1] * x_diff_flipped * y_diff_flipped
                    + src1[x + 3] * (x_diff)*y_diff_flipped
                    + src1[x + srcWidth1 + 1] * (y_diff)*x_diff_flipped
                    + src1[x + srcWidth1 + 3] * (x_diff * y_diff));
            }
        }

    }
}