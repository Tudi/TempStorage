#pragma once

struct FIBITMAP;

void ReduceColorDepth(FIBITMAP* dib, int ColorsPerChannel);
int RemoveNoColor(FIBITMAP* dib, BYTE R, BYTE G, BYTE B);
//effect of color poisoning. The edges of lines bleed into other colors
void RemoveGradient(FIBITMAP* dib);
void RemoveExtraPixels(FIBITMAP* dib);
void Errode(FIBITMAP* dib,int RequiredSameNeighbours);
void SnapColorToDominantSimilar(FIBITMAP* dib);
void BinarizeImage(FIBITMAP* dib, int ConvertToBlackBelow);
void BlurrImageToGrayScale(BYTE* Pixels, int Width, int Height, int pitch, int KernelSize);
void BlurrImageToGrayScaleIfBlack(BYTE* Pixels, int Width, int Height, int pitch, int KernelSize);
void BlurrImageToGrayScale(FIBITMAP* dib, int KernelSize);
BYTE* ImgDup(FIBITMAP* dib);
void RestoreContentFromDup(FIBITMAP* dib, BYTE* dup);
FIBITMAP* RescaleImg(FIBITMAP* dib, int NewWidth, int NewHeight);
BYTE* RescaleImg(BYTE* Pixels, int Width, int Height, int pitch, int NewWidth, int NewHeight);
__int64 Img_SAD(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int pitch2);
__int64 Img_SAD_SQ(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int pitch2);
__int64 Img_SAD(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int atx, int aty, int pitch2);
__int64 Img_SAD_FontPresent(BYTE* Pixels1, int Width1, int Height1, int pitch1, BYTE* Pixels2, int atx, int aty, int pitch2);
BYTE* RescaleImgSubPixel(BYTE* Pixels, int Width, int Height, int pitch, int NewWidth, int NewHeight);
//in a black and white image, if the pixel is not on the outside edge of a shape, remove it
void ErodeInside(BYTE* Pixels, int Width, int Height, int pitch, int ErrodeIfSmaller);
void ErodeOutside(BYTE* Pixels, int Width, int Height, int pitch, int ErrodeIfSmaller);
