#define _CRT_SECURE_NO_WARNINGS 

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include <assert.h>

#include "wind.h"

//Crop with map function
void nv12_crop(u_int8_t* src, u_int8_t* dst, int srcWidth, int srcHeight, int cropX, int cropY, int cropWidth, int cropHeight,
               int *mapX, int *mapY)
{
	int cropoffset=cropWidth*cropHeight;
	int srcoffset=srcHeight*srcWidth;
	int srcuvWidth=srcWidth/2;
	int srcuvHeight=srcHeight/2;

	int i,j; 		
	//we assume that
	assert(cropY+cropHeight<srcHeight);
	assert(cropX+cropWidth<srcWidth);

	//not using memcpy because of mapx, mapy
	for(i=0;i<cropHeight;i++)
		for(j=0;j<cropWidth;j++)
		{
			*(dst+i*cropWidth+j) = *(src+*(mapX+cropY+i)*srcWidth+*(mapY+cropX+j));
			if (((i & 1) == 0)&&((j & 1) == 0)) {
				*(dst+cropoffset+2*((i/2)*cropWidth/2+(j/2))) = *(src+srcoffset+2*(mapX[(cropY+i)]/2*srcuvWidth+mapY[(cropX+j)]/2));
				*(dst+cropoffset+2*((i/2)*cropWidth/2+(j/2))+1) = *(src+srcoffset+2*(mapX[(cropY+i)]/2*srcuvWidth+mapY[(cropX+j)]/2)+1);
			}
		}
}

//Bilinear with built-in crop
void nv12_bilinear_scale_with_crop (u_int8_t* src, u_int8_t* dst,
		int srcWidth1, int srcHeight1, int dstWidth,int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int x, y, index;
    int srcWidth=cropWidth, srcHeight=cropHeight;
    float x_ratio = ((float)(srcWidth-1))/dstWidth;
    float y_ratio = ((float)(srcHeight-1))/dstHeight;

    int srcuvWidth1=srcWidth1/2;
    int srcuvHeight1=srcHeight1/2;
    int srcoffset=srcHeight1*srcWidth1+2*(cropY/2*srcuvWidth1+cropX/2);
    int dstoffset=dstHeight*dstWidth;

    float x_diff, y_diff;
    for (int i=0;i<dstHeight-1;i++) {
        for (int j=0;j<dstWidth-1;j++) {
            x = (int)(x_ratio * j);
            y = (int)(y_ratio * i);
            x_diff = (x_ratio * j) - x;
            y_diff = (y_ratio * i) - y;
            index = ((y+cropY)*srcWidth1+x+cropX);                
            dst[i*dstWidth+j] = (int)(src[index]*(1-x_diff)*(1-y_diff) +  src[index+1]*(x_diff)*(1-y_diff) +
                    src[index+srcWidth1]*(y_diff)*(1-x_diff)   +  src[index+srcWidth1+1]*(x_diff*y_diff));
        }
    }

    for (int i=0;i<dstHeight/2-1;i++) {
        for (int j=0;j<dstWidth/2-1;j++) {
            x = (int)(x_ratio * j);
            y = (int)(y_ratio * i);
            x_diff = (x_ratio * j) - x;
            y_diff = (y_ratio * i) - y;
            index = (y*srcuvWidth1+x); 
            dst[dstoffset+2*(i*dstWidth/2+j)] = (int)(
                    src[srcoffset+2*index]*(1-x_diff)*(1-y_diff) +  src[srcoffset+2*(index+1)]*(x_diff)*(1-y_diff) +
                    src[srcoffset+2*(index+srcuvWidth1)]*(y_diff)*(1-x_diff)   +  src[srcoffset+2*(index+srcuvWidth1+1)]*(x_diff*y_diff));
            dst[dstoffset+2*(i*dstWidth/2+j)+1] = (int)(
                    src[srcoffset+2*index+1]*(1-x_diff)*(1-y_diff) +  src[srcoffset+2*(index+1)+1]*(x_diff)*(1-y_diff) +
                    src[srcoffset+2*(index+srcuvWidth1)+1]*(y_diff)*(1-x_diff)   +  src[srcoffset+2*(index+srcuvWidth1+1)+1]*(x_diff*y_diff));
        }
    }
}

//Bilinear with built-in crop with built-in map
void nv12_bilinear_scale_with_crop_with_map (u_int8_t* src, u_int8_t* dst,
		int srcWidth1, int srcHeight1, int dstWidth,int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
		int *mapX, int *mapY)
{
    int x, y, index;
    int srcWidth=cropWidth, srcHeight=cropHeight;
    float x_ratio = ((float)(srcWidth-1))/dstWidth;
    float y_ratio = ((float)(srcHeight-1))/dstHeight;

    int srcuvWidth1=srcWidth1/2;
    int srcuvHeight1=srcHeight1/2;
    int srcoffset=srcHeight1*srcWidth1;
    int dstoffset=dstHeight*dstWidth;

    float x_diff, y_diff;
    for (int i=0;i<dstHeight-1;i++) {
        for (int j=0;j<dstWidth-1;j++) {
            x = (int)(x_ratio * j);
            y = (int)(y_ratio * i);
            x_diff = (x_ratio * j) - x;
            y_diff = (y_ratio * i) - y;

            dst[i*dstWidth+j] = (int)(src[(*(mapY+y+cropY))*srcWidth1+*(mapX+x+cropX)]*(1-x_diff)*(1-y_diff) +
					src[(*(mapY+y+cropY))*srcWidth1+*(mapX+x+cropX+1)]*(x_diff)*(1-y_diff) +
                    			src[(*(mapY+y+cropY+1))*srcWidth1+*(mapX+x+cropX)]*(y_diff)*(1-x_diff) +
					src[(*(mapY+y+cropY+1))*srcWidth1+*(mapX+x+cropX+1)]*(x_diff*y_diff));
        }
    }

    for (int i=0;i<dstHeight/2-1;i++) {
        for (int j=0;j<dstWidth/2-1;j++) {
            x = (int)(x_ratio * j);
            y = (int)(y_ratio * i);
            x_diff = (x_ratio * j) - x;
            y_diff = (y_ratio * i) - y;
            dst[dstoffset+2*(i*dstWidth/2+j)] = (int)(
			src[srcoffset+2*(*(mapY+y*2+cropY)/2*srcuvWidth1+*(mapX+x*2+cropX)/2)]*(1-x_diff)*(1-y_diff) +
			src[srcoffset+2*(*(mapY+y*2+cropY)/2*srcuvWidth1+*(mapX+x*2+cropX+2)/2)]*(x_diff)*(1-y_diff) +
			src[srcoffset+2*(*(mapY+y*2+cropY+2)/2*srcuvWidth1+*(mapX+x*2+cropX)/2)]*(y_diff)*(1-x_diff) +
			src[srcoffset+2*(*(mapY+y*2+cropY+2)/2*srcuvWidth1+*(mapX+x*2+cropX+2)/2)]*(x_diff*y_diff));
            dst[dstoffset+2*(i*dstWidth/2+j)+1] = (int)(
			src[srcoffset+2*(*(mapY+y*2+cropY)/2*srcuvWidth1+*(mapX+x*2+cropX)/2)+1]*(1-x_diff)*(1-y_diff) +
			src[srcoffset+2*(*(mapY+y*2+cropY)/2*srcuvWidth1+*(mapX+x*2+cropX+2)/2)+1]*(x_diff)*(1-y_diff) +
			src[srcoffset+2*(*(mapY+y*2+cropY+2)/2*srcuvWidth1+*(mapX+x*2+cropX)/2)+1]*(y_diff)*(1-x_diff) +
			src[srcoffset+2*(*(mapY+y*2+cropY+2)/2*srcuvWidth1+*(mapX+x*2+cropX+2)/2)+1]*(x_diff*y_diff));
        }
    }
}

//Crop with map function
void nv12_bilinear_scale_with_crop_with_mapALLV2(u_char* src, u_char* dst,
	int srcWidth1, int srcHeight1, int dstWidth, int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
	int *map_all)
{
	int x, y, index, tmpx, tmpy;
	int dx, dy;
	int srcWidth = cropWidth, srcHeight = cropHeight;

	int x_ratio = (srcWidth << 8) / dstWidth;
	int y_ratio = (srcHeight << 8) / dstHeight;

	int srcuvWidth1 = srcWidth1>>1;
	int srcuvHeight1 = srcHeight1>>1;
	int dstuvWidth = dstWidth>>1;
	int dstuvHeight = dstHeight>>1;
	int srcoffset = srcHeight1 * srcWidth1;
	int dstoffset = dstHeight * dstWidth;

	tmpy = 0;
	for (int i = 0; i < dstHeight-1; i++) {
		dy = tmpy >> 8;
		y = tmpy & 0xFF;
		tmpx = 0;
		for (int j = 0; j < dstWidth-1; j++) {
			dx = tmpx >> 8;
			x = tmpx & 0xFF;
			index = (dy + cropY)*srcWidth1 + dx + cropX;

			dst[i*dstWidth + j] = ((0x100 - x) * (0x100 - y) * src[*(map_all+index)]
				+ x * (0x100 - y) * src[*(map_all+index+1)]
				+ (0x100 - x) * y * src[*(map_all+index+srcWidth1)]
				+ x * y * src[*(map_all+index+srcWidth1+1)]) >> 16;
			tmpx += x_ratio;
		}
		tmpy += y_ratio;

	}
//      Y bottom border
	tmpy = y_ratio*(dstHeight-1);
	dy = tmpy >> 8;
	y = tmpy & 0xFF;
	tmpx = 0;
	for (int j = 0; j < dstWidth; j++) {
		dx = tmpx >> 8;
		x = tmpx & 0xFF;
		index = (dy + cropY)*srcWidth1 + dx + cropX;

		dst[(dstHeight-1)*dstWidth + j] = src[*(map_all+index)];
		tmpx += x_ratio;
	}
//      Y right border
	tmpx = x_ratio*(dstWidth-1);
	dx = tmpx >> 8;
	x = tmpx & 0xFF;
	tmpy = 0;
	for (int i = 0; i < dstHeight-1; i++) {
		dy = tmpy >> 8;
		y = tmpy & 0xFF;
		index = (dy + cropY)*srcWidth1 + dx + cropX;

		dst[i*dstWidth + (dstWidth-1)] = src[*(map_all+index)];
		tmpy += y_ratio;
	}
	tmpy = 0;
	for (int i = 0; i < dstuvHeight-1; i++) {
		dy = 2*(tmpy >> 8);
		y = tmpy & 0xFF;
		tmpx = 0;
		for (int j = 0; j < dstuvWidth-1; j++) {
			dx = 2*(tmpx >> 8);
			x = tmpx & 0xFF;
			index = srcoffset + ((dy + cropY)/2 * srcuvWidth1 + (dx + cropX) / 2)*2;

			dst[dstoffset + 2 * (i*dstuvWidth + j)] =
				((0x100 - x) * (0x100 - y) * src[*(map_all+index)] +
					x * (0x100 - y) * src[*(map_all+index+2)] +
					(0x100 - x) * y * src[*(map_all+index+2*srcuvWidth1)] +
					x * y * src[*(map_all+index+2*srcuvWidth1+2)]) >> 16;

			dst[dstoffset + 2 * (i*dstuvWidth + j) + 1] =
				((0x100 - x) * (0x100 - y) * src[*(map_all+index+1)] +
					x * (0x100 - y) * src[*(map_all+index+3)] +
					(0x100 - x) * y * src[*(map_all+index+2*srcuvWidth1+1)] +
					x * y * src[*(map_all+index+2*srcuvWidth1+3)]) >> 16;
			tmpx += x_ratio;

		}
		tmpy += y_ratio;
	}

//      UV bottom border
	tmpy = y_ratio*(dstuvHeight - 1);
	dy = 2*(tmpy >> 8);
	y = tmpy & 0xFF;
	tmpx = 0;
	for (int j = 0; j < dstuvWidth; j++) {
		dx = 2*(tmpx >> 8);
		x = tmpx & 0xFF;
		index = srcoffset + 2*((dy + cropY) / 2 * srcuvWidth1 + (dx + cropX) / 2);
		
		dst[dstoffset + 2 * ((dstuvHeight - 1)*dstuvWidth + j)] = src[*(map_all+index)];
		dst[dstoffset + 2 * ((dstuvHeight - 1)*dstuvWidth + j) + 1] = src[*(map_all+index+1)];
		tmpx += x_ratio;

	}

//      UV right border
	tmpy = 0;
	tmpx = x_ratio*(dstuvWidth-1);
	dx = 2*(tmpx >> 8);
	x = tmpx & 0xFF;
	for (int i = 0; i < dstuvHeight-1; i++) {
		dy = 2*(tmpy >> 8);
		y = tmpy & 0xFF;
		index = srcoffset + 2*((dy + cropY) /2 * srcuvWidth1 + (dx + cropX) / 2);

		dst[dstoffset + 2 * (i*dstuvWidth + dstuvWidth - 1)] = src[*(map_all+index)];
		dst[dstoffset + 2 * (i*dstuvWidth + dstuvWidth - 1) + 1] = src[*(map_all+index+1)];
		tmpy += y_ratio;
	}
}


//Bilinear with built-in crop
void nv12_bilinear_scale_with_cropV2 (u_int8_t* src, u_int8_t* dst,
		int srcWidth1, int srcHeight1, int dstWidth,int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight)
{
    int x, y, index, tmpx, tmpy, dx, dy;
    int srcWidth=cropWidth, srcHeight=cropHeight;
    int x_ratio = (srcWidth << 8)/dstWidth;
    int y_ratio = (srcHeight << 8)/dstHeight;

    int srcuvWidth1=srcWidth1/2;
    int srcuvHeight1=srcHeight1/2;
    int dstuvWidth=dstWidth/2;
    int dstuvHeight=dstHeight/2;

    int srcoffset=srcHeight1*srcWidth1+2*(cropY/2*srcuvWidth1+cropX/2);
    int dstoffset=dstHeight*dstWidth;
//  Y  
    tmpy=0;
    for (int i=0;i<dstHeight-1;i++) {
        dy = tmpy >> 8;
	y = tmpy & 0xFF;
        tmpx=0;
        for (int j=0;j<dstWidth-1;j++) {
            dx = tmpx >> 8;
	    x = tmpx & 0xFF;
            index = ((dy+cropY)*srcWidth1+dx+cropX);
            dst[i*dstWidth+j] = ((0x100 - x) * (0x100 - y) * src[index]
			                  + x * (0x100 - y) * src[index+1]
			                  + (0x100 - x) * y * src[index+srcWidth1]
			                  + x * y * src[index+srcWidth1+1])>>16;
            tmpx += x_ratio;
        }
        tmpy += y_ratio;

    }
//  bottom border Y
    tmpy=y_ratio*(dstHeight-1);
    dy = tmpy >> 8;
    y = tmpy & 0xFF;
    tmpx=0;
    for (int j=0;j<dstWidth;j++) {
        dx = tmpx >> 8;
	x = tmpx & 0xFF;
        index = ((dy+cropY)*srcWidth1+dx+cropX);
        dst[(dstHeight-1)*dstWidth+j] = src[index];
        tmpx += x_ratio;
    }
//right border Y
    tmpy=0;
    tmpx=x_ratio*(dstWidth-1);
    dx = tmpx >> 8;
    x = tmpx & 0xFF;
    for (int i=0;i<dstHeight;i++) {
        dy = tmpy >> 8;
	y = tmpy & 0xFF;
        index = ((dy+cropY)*srcWidth1+dx+cropX);
        dst[i*dstWidth+dstWidth-1] = src[index];
        tmpy += y_ratio;
    }
//  UV
    tmpy=0;
    for (int i=0;i<dstuvHeight-1;i++) {
        dy = tmpy >> 8;
	y = tmpy & 0xFF;
        tmpx=0;
        for (int j=0;j<dstuvWidth-1;j++) {
            dx = tmpx >> 8;
	    x = tmpx & 0xFF;
            index = (dy*srcuvWidth1+dx); 

            dst[dstoffset+2*(i*dstuvWidth+j)] =
                    ((0x100 - x) * (0x100 - y) * src[srcoffset+2*index] +
                    x * (0x100 - y) * src[srcoffset+2*(index+1)] +
                    (0x100 - x) * y * src[srcoffset+2*(index+srcuvWidth1)] +
                    x * y * src[srcoffset+2*(index+srcuvWidth1+1)])>>16;
            dst[dstoffset+2*(i*dstuvWidth+j)+1] = 
                    ((0x100 - x) * (0x100 - y) * src[srcoffset+2*index+1] +
                    x * (0x100 - y) * src[srcoffset+2*(index+1)+1] +
                    (0x100 - x) * y * src[srcoffset+2*(index+srcuvWidth1)+1] +
                    x * y * src[srcoffset+2*(index+srcuvWidth1+1)+1])>>16;
            tmpx += x_ratio;

        }
        tmpy += y_ratio;
    }
    //bottom border UV
    tmpy=y_ratio*(dstuvHeight-1);
    dy = tmpy >> 8;
    y = tmpy & 0xFF;
    tmpx=0;
    for (int j=0;j<dstuvWidth;j++) {
        dx = tmpx >> 8;
	x = tmpx & 0xFF;
        index = (dy*srcuvWidth1+dx); 

        dst[dstoffset+2*((dstuvHeight-1)*dstuvWidth+j)] = src[srcoffset+2*index];
        dst[dstoffset+2*((dstuvHeight-1)*dstuvWidth+j)+1] = src[srcoffset+2*index+1];
        tmpx += x_ratio;

    }
    //right border UV
    tmpy=0;
    tmpx=x_ratio*(dstuvWidth-1);
    dx = tmpx >> 8;
    x = tmpx & 0xFF;
    for (int i=0;i<dstuvHeight;i++) {
        dy = tmpy >> 8;
	y = tmpy & 0xFF;
        index = (dy*srcuvWidth1+dx); 

        dst[dstoffset+2*(i*dstuvWidth+dstuvWidth-1)] = src[srcoffset+2*index];
        dst[dstoffset+2*(i*dstuvWidth+dstuvWidth-1)+1] = src[srcoffset+2*index+1];
        tmpy += y_ratio;
    }

}


//Bilinear with built-in crop with built-in map V2
void nv12_bilinear_scale_with_crop_with_mapV2 (u_int8_t* src, u_int8_t* dst,
		int srcWidth1, int srcHeight1, int dstWidth,int dstHeight, int cropX, int cropY, int cropWidth, int cropHeight,
		int *mapX, int *mapY)
{
    int x, y, index, tmpx, tmpy;
    int dx, dy;
    int srcWidth=cropWidth, srcHeight=cropHeight;

    int x_ratio = (srcWidth << 8)/dstWidth;
    int y_ratio = (srcHeight << 8)/dstHeight;

    int srcuvWidth1=srcWidth1/2;
    int srcuvHeight1=srcHeight1/2;
    int srcoffset=srcHeight1*srcWidth1;
    int dstoffset=dstHeight*dstWidth;

    tmpy=0;
    for (int i=0;i<dstHeight-1;i++) {
        dy = tmpy >> 8;
	y = tmpy & 0xFF;
        tmpx=0;
        for (int j=0;j<dstWidth-1;j++) {
            dx = tmpx >> 8;
	    x = tmpx & 0xFF;
            dst[i*dstWidth+j] = ((0x100 - x) * (0x100 - y) * src[(*(mapY+dy+cropY))*srcWidth1+*(mapX+dx+cropX)]
			                  + x * (0x100 - y) * src[(*(mapY+dy+cropY))*srcWidth1+*(mapX+dx+cropX+1)]
			                  + (0x100 - x) * y * src[(*(mapY+dy+cropY+1))*srcWidth1+*(mapX+dx+cropX)]
			                  + x * y * src[(*(mapY+dy+cropY+1))*srcWidth1+*(mapX+dx+cropX+1)])>>16;
            tmpx += x_ratio;
        }
        tmpy += y_ratio;
    }

    tmpy=0;
    for (int i=0;i<dstHeight/2-1;i++) {
        dy = 2*(tmpy >> 8);
	y = tmpy & 0xFF;
        tmpx=0;

        for (int j=0;j<dstWidth/2-1;j++) {
            dx = 2*(tmpx >> 8);
	    x = tmpx & 0xFF;
            dst[dstoffset+2*(i*dstWidth/2+j)] = 
				((0x100 - x) * (0x100 - y) * src[srcoffset+2*(*(mapY+dy+cropY)/2*srcuvWidth1+*(mapX+dx+cropX)/2)]
				+ x * (0x100 - y) * src[srcoffset+2*(*(mapY+dy+cropY)/2*srcuvWidth1+*(mapX+dx+cropX+2)/2)]
				+ (0x100 - x) * y * src[srcoffset+2*(*(mapY+dy+cropY+2)/2*srcuvWidth1+*(mapX+dx+cropX)/2)]
				+ x * y * src[srcoffset+2*(*(mapY+dy+cropY+2)/2*srcuvWidth1+*(mapX+dx+cropX+2)/2)])>>16;
            dst[dstoffset+2*(i*dstWidth/2+j)+1] = ((0x100 - x) * (0x100 - y) * src[srcoffset+2*(*(mapY+dy+cropY)/2*srcuvWidth1+*(mapX+dx+cropX)/2)+1]
			                  + x * (0x100 - y) * src[srcoffset+2*(*(mapY+dy+cropY)/2*srcuvWidth1+*(mapX+dx+cropX+2)/2)+1]
			                  + (0x100 - x) * y * src[srcoffset+2*(*(mapY+dy+cropY+2)/2*srcuvWidth1+*(mapX+dx+cropX)/2)+1]
			                  + x * y * src[srcoffset+2*(*(mapY+dy+cropY+2)/2*srcuvWidth1+*(mapX+dx+cropX+2)/2)+1])>>16;
            tmpx += x_ratio;
        }
        tmpy += y_ratio;
    }
}

//Simple bilinear 
void nv12_bilinear_scale (u_int8_t* src, u_int8_t* dst,
		int srcWidth, int srcHeight, int dstWidth,int dstHeight)
{
    int x, y, index ;
    float x_ratio = ((float)(srcWidth-1))/dstWidth ;
    float y_ratio = ((float)(srcHeight-1))/dstHeight ;
    float x_ratio1 = ((float)(srcWidth/2-1))/dstWidth/2 ;
    float y_ratio1 = ((float)(srcHeight/2-1))/dstHeight/2 ;

    int srcoffset=srcHeight*srcWidth;
    int dstoffset=dstHeight*dstWidth;
    int srcuvWidth=srcWidth/2;
    int srcuvHeight=srcHeight/2;

    float x_diff, y_diff, blue, red, green ;
    for (int i=0;i<dstHeight;i++) {
        for (int j=0;j<dstWidth;j++) {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;
            index = (y*srcWidth+x) ;                
            dst[i*dstWidth+j] = (int)(
                    src[index]*(1-x_diff)*(1-y_diff) +  src[index+1]*(x_diff)*(1-y_diff) +
                    src[index+srcWidth]*(y_diff)*(1-x_diff)   +  src[index+srcWidth+1]*(x_diff*y_diff));
        }
    }

    for (int i=0;i<dstHeight/2;i++) {
        for (int j=0;j<dstWidth/2;j++) {
            x = (int)(x_ratio * j) ;
            y = (int)(y_ratio * i) ;
            x_diff = (x_ratio * j) - x ;
            y_diff = (y_ratio * i) - y ;
            index = (y*srcuvWidth+x) ; 
            dst[dstoffset+2*(i*dstWidth/2+j)] = (int)(
                    src[srcoffset+2*index]*(1-x_diff)*(1-y_diff) +  src[srcoffset+2*(index+1)]*(x_diff)*(1-y_diff) +
                    src[srcoffset+2*(index+srcuvWidth)]*(y_diff)*(1-x_diff)   +  src[srcoffset+2*(index+srcuvWidth+1)]*(x_diff*y_diff));
            dst[dstoffset+2*(i*dstWidth/2+j)+1] = (int)(
                    src[srcoffset+2*index+1]*(1-x_diff)*(1-y_diff) +  src[srcoffset+2*(index+1)+1]*(x_diff)*(1-y_diff) +
                    src[srcoffset+2*(index+srcuvWidth)+1]*(y_diff)*(1-x_diff)   +  src[srcoffset+2*(index+srcuvWidth+1)+1]*(x_diff*y_diff));
        }
    }
}

static inline double cubicInterpolate (double p0, double p1, double p2, double p3, double x) {
	return p1 + 0.5 * x*(p2 - p0 + x*(2.0*p0 - 5.0*p1 + 4.0*p2 - p3 + x*(3.0*(p1 - p2) + p3 - p0)));
}

//Simple bicubic
void nv12_bicubic_scale(u_int8_t* src, u_int8_t* dst, int srcWidth, int srcHeight, int dstWidth, int dstHeight)
{
	int x, y, x2, y2, index, index2;
	float x_ratio = ((float)(srcWidth-1))/dstWidth ;
	float y_ratio = ((float)(srcHeight-1))/dstHeight ;
	float col0, col1, col2, col3, value;

	float x_diff, y_diff, x_diff2, y_diff2;
	int srcoffset = 0, dstoffset=0;
	int i,j;

	srcoffset=srcHeight*srcWidth;
	dstoffset=dstHeight*dstWidth;
	int srcuvWidth=srcWidth/2;
	int srcuvHeight=srcHeight/2;
	for (i=1;i<dstHeight;i++) {
		for (j=1;j<dstWidth;j++) {

			x = (int)(x_ratio * j) ;
			y = (int)(y_ratio * i) ;
			if (x<1) x=1;
			if (y<1) y=1;

			x_diff = (x_ratio * j) - x ;
			y_diff = (y_ratio * i) - y ;
			index = (y*srcWidth+x) ;                

			// bi-cubic interpolation
			col0 = cubicInterpolate(*(src+index - 1 - srcWidth), *(src+index  - srcWidth), *(src+index + 1 - srcWidth), *(src+index + 2 - srcWidth), x_diff);

			col1 = cubicInterpolate(*(src+index - 1), *(src+index), *(src+index + 1), *(src+index + 2), x_diff);

			col2 = cubicInterpolate(*(src+index - 1 + srcWidth), *(src+index + srcWidth), *(src+index + 1 + srcWidth), *(src+index + 2 + srcWidth), x_diff);

			col3 = cubicInterpolate(*(src+index - 1 + 2*srcWidth), *(src+index + 2*srcWidth), *(src+index + 1 + 2*srcWidth), *(src+index + 2 + 2*srcWidth), x_diff);
			value = cubicInterpolate(col0, col1, col2, col3, y_diff);
			// Clamp the values since the curve can put the value below 0 or above 255
			if (value > 255.0f) value=255.0f;
			if (value < .0f) value=.0f;
			dst[i*dstWidth+j] = (u_int8_t)value;

			if (((i & 1) == 0)&&((j & 1) == 0)) {
				x2 = (int)(x_ratio * j/2) ;
				y2 = (int)(y_ratio * i/2) ;
				if (x2<1) x2=1;
				if (y2<1) y2=1;

				x_diff2 = (x_ratio * j/2) - x2 ;
				y_diff2 = (y_ratio * i/2) - y2 ;

				index2 = (y2*srcuvWidth+x2) ; 
				//the same for u-plane
				col0 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 - srcuvWidth)), *(src+srcoffset+2*(index2 - srcuvWidth)), *(src+srcoffset+2*(index2 + 1 - srcuvWidth)), *(src+srcoffset+2*(index2 + 2 - srcuvWidth)), x_diff2);
				col1 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1)), *(src+srcoffset+2*index2), *(src+srcoffset+2*(index2 + 1)), *(src+srcoffset+2*(index2 + 2)), x_diff2);
				col2 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcuvWidth)), *(src+srcoffset+2*(index2 + srcuvWidth)), *(src+srcoffset+2*(index2 + 1 + srcuvWidth)), *(src+srcoffset+2*(index2 + 2 + srcuvWidth)), x_diff2);
				col3 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcWidth)), *(src+srcoffset+2*(index2 + srcWidth)), *(src+srcoffset+2*(index2 + 1 + srcWidth)), *(src+srcoffset+2*(index2 + 2 + srcWidth)), x_diff2);
				value = cubicInterpolate(col0, col1, col2, col3, y_diff2);
				if (value > 255.0f) value=255.0f;
				if (value < .0f) value=.0f;

				//the same for v-plane
				dst[dstoffset+2*(i/2*dstWidth/2+j/2)] = (u_int8_t)value;
				col0 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 - srcuvWidth)+1), *(src+srcoffset+2*(index2 - srcuvWidth)+1), *(src+srcoffset+2*(index2 + 1 - srcuvWidth)+1), *(src+srcoffset+2*(index2 + 2 - srcuvWidth)+1), x_diff2);
				col1 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1)+1), *(src+srcoffset+2*index2+1), *(src+srcoffset+2*(index2 + 1)+1), *(src+srcoffset+2*(index2 + 2)+1), x_diff2);
				col2 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcuvWidth)+1), *(src+srcoffset+2*(index2 + srcuvWidth)+1), *(src+srcoffset+2*(index2 + 1 + srcuvWidth)+1), *(src+srcoffset+2*(index2 + 2 + srcuvWidth)+1), x_diff2);
				col3 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcWidth)+1), *(src+srcoffset+2*(index2 + srcWidth)+1), *(src+srcoffset+2*(index2 + 1 + srcWidth)+1), *(src+srcoffset+2*(index2 + 2 + srcWidth)+1), x_diff2);
				value = cubicInterpolate(col0, col1, col2, col3, y_diff2);
				if (value > 255.0f) value=255.0f;
				if (value < .0f) value=.0f;
				dst[dstoffset+2*(i/2*dstWidth/2+j/2)+1] = (u_int8_t)value;
			}	
		}
	}	
}

//Bicubic with built-in crop
void nv12_bicubic_scale_with_crop(u_int8_t* src, u_int8_t* dst, int srcWidth1, int srcHeight1, int dstWidth, int dstHeight,
				  int cropX, int cropY, int cropWidth, int cropHeight)
{
	int srcWidth=cropWidth, srcHeight=cropHeight;
	int x, y, x2, y2, index, index2;
	float x_ratio = ((float)(srcWidth-1))/dstWidth ;
	float y_ratio = ((float)(srcHeight-1))/dstHeight ;
	float col0, col1, col2, col3, value;

	float x_diff, y_diff, x_diff2, y_diff2;
	int srcoffset = 0, dstoffset=0;
	int i,j;

	int srcuvWidth1=srcWidth1/2;
	int srcuvHeight1=srcHeight1/2;
	srcoffset=srcHeight1*srcWidth1+2*(cropY/2*srcuvWidth1+cropX/2);
	dstoffset=dstHeight*dstWidth;
	
	for (i=1;i<dstHeight;i++) {
		for (j=1;j<dstWidth;j++) {

			x = (int)(x_ratio * j) ;
			y = (int)(y_ratio * i) ;
			if (x<1) x=1;
			if (y<1) y=1;

			x_diff = (x_ratio * j) - x ;
			y_diff = (y_ratio * i) - y ;
			index = ((y+cropY)*srcWidth1+x+cropX);                

			// bi-cubic interpolation
			col0 = cubicInterpolate(*(src+index - 1 - srcWidth1), *(src+index  - srcWidth1), *(src+index + 1 - srcWidth1), *(src+index + 2 - srcWidth1), x_diff);

			col1 = cubicInterpolate(*(src+index - 1), *(src+index), *(src+index + 1), *(src+index + 2), x_diff);

			col2 = cubicInterpolate(*(src+index - 1 + srcWidth1), *(src+index + srcWidth1), *(src+index + 1 + srcWidth1), *(src+index + 2 + srcWidth1), x_diff);

			col3 = cubicInterpolate(*(src+index - 1 + 2*srcWidth1), *(src+index + 2*srcWidth1), *(src+index + 1 + 2*srcWidth1), *(src+index + 2 + 2*srcWidth1), x_diff);
			value = cubicInterpolate(col0, col1, col2, col3, y_diff);
			// Clamp the values since the curve can put the value below 0 or above 255
			if (value > 255.0f) value=255.0f;
			if (value < .0f) value=.0f;
			dst[i*dstWidth+j] = (u_int8_t)value;

			if (((i & 1) == 0)&&((j & 1) == 0)) {
				x2 = (int)(x_ratio * j/2) ;
				y2 = (int)(y_ratio * i/2) ;
				if (x2<1) x2=1;
				if (y2<1) y2=1;

				x_diff2 = (x_ratio * j/2) - x2 ;
				y_diff2 = (y_ratio * i/2) - y2 ;

				index2 = (y2*srcuvWidth1+x2) ; 
				//the same for u-plane
				col0 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 - srcuvWidth1)), *(src+srcoffset+2*(index2 - srcuvWidth1)), *(src+srcoffset+2*(index2 + 1 - srcuvWidth1)), *(src+srcoffset+2*(index2 + 2 - srcuvWidth1)), x_diff2);
				col1 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1)), *(src+srcoffset+2*index2), *(src+srcoffset+2*(index2 + 1)), *(src+srcoffset+2*(index2 + 2)), x_diff2);
				col2 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcuvWidth1)), *(src+srcoffset+2*(index2 + srcuvWidth1)), *(src+srcoffset+2*(index2 + 1 + srcuvWidth1)), *(src+srcoffset+2*(index2 + 2 + srcuvWidth1)), x_diff2);
				col3 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcWidth1)), *(src+srcoffset+2*(index2 + srcWidth1)), *(src+srcoffset+2*(index2 + 1 + srcWidth1)), *(src+srcoffset+2*(index2 + 2 + srcWidth1)), x_diff2);
				value = cubicInterpolate(col0, col1, col2, col3, y_diff2);
				if (value > 255.0f) value=255.0f;
				if (value < .0f) value=.0f;

				//the same for v-plane
				dst[dstoffset+2*(i/2*dstWidth/2+j/2)] = (u_int8_t)value;
				col0 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 - srcuvWidth1)+1), *(src+srcoffset+2*(index2 - srcuvWidth1)+1), *(src+srcoffset+2*(index2 + 1 - srcuvWidth1)+1), *(src+srcoffset+2*(index2 + 2 - srcuvWidth1)+1), x_diff2);
				col1 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1)+1), *(src+srcoffset+2*index2+1), *(src+srcoffset+2*(index2 + 1)+1), *(src+srcoffset+2*(index2 + 2)+1), x_diff2);
				col2 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcuvWidth1)+1), *(src+srcoffset+2*(index2 + srcuvWidth1)+1), *(src+srcoffset+2*(index2 + 1 + srcuvWidth1)+1), *(src+srcoffset+2*(index2 + 2 + srcuvWidth1)+1), x_diff2);
				col3 = cubicInterpolate(*(src+srcoffset+2*(index2 - 1 + srcWidth1)+1), *(src+srcoffset+2*(index2 + srcWidth1)+1), *(src+srcoffset+2*(index2 + 1 + srcWidth1)+1), *(src+srcoffset+2*(index2 + 2 + srcWidth1)+1), x_diff2);
				value = cubicInterpolate(col0, col1, col2, col3, y_diff2);
				if (value > 255.0f) value=255.0f;
				if (value < .0f) value=.0f;
				dst[dstoffset+2*(i/2*dstWidth/2+j/2)+1] = (u_int8_t)value;
			}	
		}
	}	
}

//Bicubic with built-in crop with built-in map
void nv12_bicubic_scale_with_crop_with_map(u_int8_t* src, u_int8_t* dst, int srcWidth1, int srcHeight1, int dstWidth, int dstHeight,
				  int cropX, int cropY, int cropWidth, int cropHeight, int *mapX, int *mapY)
{
	int srcWidth=cropWidth, srcHeight=cropHeight;
	int x, y, x2, y2, index, index2;
	float x_ratio = ((float)(srcWidth-1))/dstWidth ;
	float y_ratio = ((float)(srcHeight-1))/dstHeight ;
	float col0, col1, col2, col3, value;

	float x_diff, y_diff, x_diff2, y_diff2;
	int srcoffset = 0, dstoffset=0;
	int i,j;

	int srcuvWidth1=srcWidth1/2;
	int srcuvHeight1=srcHeight1/2;
	srcoffset=srcHeight1*srcWidth1;//+2*(cropY/2*srcuvWidth1+cropX/2); //!!!
	dstoffset=dstHeight*dstWidth;
	
	for (i=1;i<dstHeight;i++) {
		for (j=1;j<dstWidth;j++) {

			x = (int)(x_ratio * j) ;
			y = (int)(y_ratio * i) ;
			if (x<1) x=1;
			if (y<1) y=1;

			x_diff = (x_ratio * j) - x ;
			y_diff = (y_ratio * i) - y ;
			//index = ((y+cropY)*srcWidth1+x+cropX);                

			// bi-cubic interpolation
			col0 = cubicInterpolate(
						*(src+*(mapY+y+cropY-1)*srcWidth1 + *(mapX + x + cropX - 1)), 
						*(src+*(mapY+y+cropY-1)*srcWidth1 + *(mapX + x + cropX)), 
						*(src+*(mapY+y+cropY-1)*srcWidth1 + *(mapX + x + cropX + 1)), 
						*(src+*(mapY+y+cropY-1)*srcWidth1 + *(mapX + x + cropX + 2)), x_diff);

			col1 = cubicInterpolate(
						*(src+*(mapY+y+cropY)*srcWidth1 + *(mapX + x + cropX - 1)), 
						*(src+*(mapY+y+cropY)*srcWidth1 + *(mapX + x + cropX)), 
						*(src+*(mapY+y+cropY)*srcWidth1 + *(mapX + x + cropX + 1)), 
						*(src+*(mapY+y+cropY)*srcWidth1 + *(mapX + x + cropX + 2)), x_diff);
			col2 = cubicInterpolate(
						*(src+*(mapY+y+cropY+1)*srcWidth1 + *(mapX + x + cropX - 1)), 
						*(src+*(mapY+y+cropY+1)*srcWidth1 + *(mapX + x + cropX)), 
						*(src+*(mapY+y+cropY+1)*srcWidth1 + *(mapX + x + cropX + 1)), 
						*(src+*(mapY+y+cropY+1)*srcWidth1 + *(mapX + x + cropX + 2)), x_diff);
			col3 = cubicInterpolate(
						*(src+*(mapY+y+cropY+2)*srcWidth1 + *(mapX + x + cropX - 1)), 
						*(src+*(mapY+y+cropY+2)*srcWidth1 + *(mapX + x + cropX)), 
						*(src+*(mapY+y+cropY+2)*srcWidth1 + *(mapX + x + cropX + 1)), 
						*(src+*(mapY+y+cropY+2)*srcWidth1 + *(mapX + x + cropX + 2)), x_diff);
			value = cubicInterpolate(col0, col1, col2, col3, y_diff);
			// Clamp the values since the curve can put the value below 0 or above 255
			if (value > 255.0f) value=255.0f;
			if (value < .0f) value=.0f;
			dst[i*dstWidth+j] = (u_int8_t)value;

			if (((i & 1) == 0)&&((j & 1) == 0)) {
				x2 = (int)(x_ratio * j) ;
				y2 = (int)(y_ratio * i) ;
				if (x2<1) x2=1;
				if (y2<1) y2=1;

				x_diff2 = (x_ratio * j) - x2 ;
				y_diff2 = (y_ratio * i) - y2 ;

				//for u-plane
				col0 = cubicInterpolate(
						*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)), 
						x_diff2);
				col1 = cubicInterpolate(
						*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)), 
						x_diff2);
				col2 = cubicInterpolate(
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)), 
						x_diff2);
				col3 = cubicInterpolate(
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)), 
						*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)), 
						x_diff2);
				value = cubicInterpolate(col0, col1, col2, col3, y_diff2);
				if (value > 255.0f) value=255.0f;
				if (value < .0f) value=.0f;

				//the same for v-plane
				dst[dstoffset+2*(i/2*dstWidth/2+j/2)] = (u_int8_t)value;
				col0 = cubicInterpolate(
					*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY - 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)+1), 
					x_diff2);
				col1 = cubicInterpolate(
					*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)+1), 
					x_diff2);
				col2 = cubicInterpolate(
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 2)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)+1), 
					x_diff2);
				col3 = cubicInterpolate(
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX - 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX + 2)/2)+1), 
					*(src+srcoffset+2*(*(mapY + y2 + cropY + 4)/2*srcuvWidth1 + *(mapX + x2 + cropX + 4)/2)+1), 
					x_diff2);
				value = cubicInterpolate(col0, col1, col2, col3, y_diff2);
				if (value > 255.0f) value=255.0f;
				if (value < .0f) value=.0f;
				dst[dstoffset+2*(i/2*dstWidth/2+j/2)+1] = (u_int8_t)value;
			}	
		}
	}	
}

int ImageProcess(u_int8_t * src, u_int8_t* dst, int sw,int sh,int dw,int dh, int xcrop, int ycrop, int cw, int ch,
		 int *mapX, int *mapY)
{
	if( (src == NULL) || (dst == NULL) || (0 == dw) || (0 == dh) || (0 == sw) || (0 == sh)|| (0 == xcrop) || (0 == ycrop)|| (0 == cw) || (0 == ch))
	{
	    printf("params error\n");
	    return -1;
	}


	u_int8_t* crop = (u_int8_t*)malloc(cw*ch*3/2);

	nv12_crop(src, crop, sw, sh, xcrop, ycrop, cw, ch, mapX, mapY); 
	nv12_bicubic_scale(crop, dst, cw, ch, dw, dh);
	//nv12_bilinear_scale(crop, dst, cw, ch, dw, dh);


	free(crop);
	crop=NULL;
	return 0;
}
int main(int argc,char**argv)
{
	setbuf(stdout, NULL);

	if(argc!=11)
	{
	    printf("Input Error!\n");
	    printf("Usage :  <Input NV12file> <Output NV12file> <sw> <sh> <dw> <dh> <cropx> <cropy> <cw> <ch>");
  		return 0;
	}
 
	FILE *inputfp = NULL;
	FILE *outputfp = NULL;
 
	inputfp = fopen(argv[1], "rb");
	if (!inputfp)
	{
	    fprintf(stderr, "fopen failed for input file[%s]\n",argv[1]);
	    return -1;
	}
 
	outputfp = fopen(argv[2], "wb");
 
	if (!outputfp)
	{
	    fprintf(stderr, "fopen failed for output file[%s]\n",argv[2]);
	    return -1;
	}
 
	int sw = atoi(argv[3]);
	int sh = atoi(argv[4]);
	int dw = atoi(argv[5]);
	int dh = atoi(argv[6]);
	int cropx = atoi(argv[7]);
	int cropy = atoi(argv[8]);
	int cw = atoi(argv[9]);
	int ch = atoi(argv[10]);


	if(sw <= 0 || sh <= 0 || dw <= 0 || dh <=0)
	{
	    fprintf(stderr, "parameter error [sw= %d,sh= %d,dw= %d,dh= %d]\n",sw,sh,dw,dh);
	    return -1;
	}
 
	int inPixels = sw * sh * 3/2;
	int outPixels = dw * dh * 3/2;
 
	u_int8_t* pInBuffer = (u_int8_t*)malloc(inPixels);
	fread(pInBuffer,1,inPixels,inputfp);
	u_int8_t* pOutBuffer = (u_int8_t*)malloc(outPixels);
	u_int8_t* pOutBuffer2 = (u_int8_t*)malloc(outPixels);


	//init mapx, mapy transformation
	int* pmapy = (int*)malloc(sw*sizeof(int));
	int* pmapx = (int*)malloc(sh*sizeof(int));
	int* pmap_all = (int*)malloc(sh*sw*3/2*sizeof(int));
	int i, j;

	//Define the transformation rule here
	for(i=0;i<sh;i++) *(pmapx+i)=sh-i;
	for(j=0;j<sw;j++) pmapy[j]=sw-j;

	for(i=0;i<sh;i++) 
		for(j=0;j<sw;j++)
			pmap_all[sw*i+j]=sw*(sh-i)+sw-j; 
	for(i=0;i<sh/2;i++) 
		for(j=0;j<sw/2;j++) {
			pmap_all[sw*sh+2*(sw/2*i+j)]=sw*sh+2*(sw/2*(sh/2-i)+sw/2-j); 
			pmap_all[sw*sh+2*(sw/2*i+j)+1]=sw*sh+2*(sw/2*(sh/2-i)+sw/2-j)+1; 
		}

	u_int64_t delta_us1 = 0, delta_us2 = 0;
	struct timespec start, end;
	for (int InterleavedTest = 0; InterleavedTest < MultiRunCount; InterleavedTest++)
	{
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);

		//#1 Processing with {cropping and mapping} once outside the resize function
		for (int i = 0; i < TestRepeatCount; i++)
//			nv12_bilinear_scale_with_crop(pInBuffer, pOutBuffer2, sw, sh, dw, dh, cropx, cropy, cw, ch);
			nv12_bilinear_scale_with_crop_with_mapALLV2(pInBuffer, pOutBuffer2, sw, sh, dw, dh, cropx, cropy, cw, ch, pmap_all);
		//ImageProcess(pInBuffer,pOutBuffer,sw,sh,dw,dh,cropx,cropy,cw,ch,pmapx, pmapy);
		clock_gettime(CLOCK_MONOTONIC_RAW, &end);

		u_int64_t delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
		delta_us1 += delta_us;
		clock_gettime(CLOCK_MONOTONIC_RAW, &start);

		//#2 Processing resize with crop and resize without allocation additional memory
		for (int i = 0; i < TestRepeatCount; i++)
			nv12_bilinear_scale_with_crop_with_map_(pInBuffer, pOutBuffer2, sw, sh, dw, dh, cropx, cropy, cw, ch, pmap_all);

		clock_gettime(CLOCK_MONOTONIC_RAW, &end);

		delta_us = (end.tv_sec - start.tv_sec) * 1000000 + (end.tv_nsec - start.tv_nsec) / 1000;
		delta_us2 += delta_us;
	}

	printf("\nTime elapsed with crop before resize, %lld\n", delta_us1 / 1000);
	printf("\nTime elapsed for built-in crop, %lld\n", delta_us2 / 1000);

	//Write the result of #2 to file
	fwrite(pOutBuffer2, 1 , outPixels, outputfp);
 
	free(pmapx);
	free(pmapy);
	pmapx=NULL;
	pmapy=NULL;
	free(pInBuffer);
	free(pOutBuffer);
	free(pOutBuffer2);
	fclose(inputfp);
	fclose(outputfp);
	pInBuffer = NULL;
	pOutBuffer = NULL;
	pOutBuffer2 = NULL;
	inputfp = NULL;
	outputfp = NULL;
	return 0;
}

