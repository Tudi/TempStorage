#include "StdAfx.h"

FIBITMAP* openImage(const char* fileName)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
	const char* InputFileName = fileName;
	if (InputFileName == NULL)
		InputFileName = "../../page_fetcher/images/cryptopunks/8314.png";
	dib = LoadImage_(InputFileName);

	if (dib == NULL)
	{
		printf("Could not open input file %s\n", InputFileName);
		return NULL;
	}

	int bpp = FreeImage_GetBPP(dib);
	if (bpp != Bytespp * 8)
		dib = FreeImage_ConvertTo24Bits(dib);
	bpp = FreeImage_GetBPP(dib);
	if (bpp != Bytespp * 8)
	{
		printf("!!!!Only support 24 bpp input. Upgrade software or convert input from %d to 24\n", bpp);
		return NULL;
	}

	return dib;
}

int printHashesOf1Image(const char *fileName)
{
	FIBITMAP* dib = openImage(fileName);

	A_HASH_RGB aHash;
	aHashInit(&aHash, 64);
	genAHashGrayScale(dib, &aHash);

	A_HASH_RGB pHash;
	aHashInit(&pHash, 64);
	genPHashGrayScale(dib, &pHash);

	printf("{\"AHASH\":\"");
	printAHash(&aHash, 1);
	printf("\",\"PHASH\":\"");
	printAHash(&pHash, 1);
	printf("\"}");

	// free the dib
	FreeImage_Unload(dib);

	return 0;
}

int printImageSimilarityScore(const char* fileName1, const char* fileName2)
{
	FIBITMAP* dib1 = openImage(fileName1);
	FIBITMAP* dib2 = openImage(fileName2);

	A_HASH_RGB aHash1;
	aHashInit(&aHash1, 64);
	genAHashGrayScale(dib1, &aHash1);
	A_HASH_RGB pHash1;
	aHashInit(&pHash1, 64);
	genPHashGrayScale(dib1, &pHash1);

	A_HASH_RGB aHash2;
	aHashInit(&aHash2, 64);
	genAHashGrayScale(dib2, &aHash2);
	A_HASH_RGB pHash2;
	aHashInit(&pHash2, 64);
	genPHashGrayScale(dib2, &pHash2);

	A_HASH_RGB_CompareResult aHash, pHash;
	compareHash(&aHash1, &aHash2, &aHash);
	compareHash(&pHash1, &pHash2, &pHash);

	double overallMatchChance = MAX(aHash.pctMatchAvg, pHash.pctMatchAvg);
	printf("%f match chance\n", overallMatchChance);
	printf("%f A match chance\n", aHash.pctMatchAvg);
	printf("%f P match chance\n", pHash.pctMatchAvg);

	// free the dib
	FreeImage_Unload(dib1);
	FreeImage_Unload(dib2);

	return 0;

}

int main(int argc, char **argv)
{
//	printImageSimilarityScore("../../page_fetcher/images/cryptopunks/8300.png", "../../image_manip/resize/8300.png");
//	printImageSimilarityScore("../../page_fetcher/images/cryptopunks/8300.png", "../../image_manip/blurr/8300.png");
	if (argc == 1)
		printHashesOf1Image(NULL);
	else if(argc == 2)
		printHashesOf1Image(argv[1]);
	else if(argc == 3 && argv[1] != NULL && argv[2] != NULL)
		printImageSimilarityScore(argv[1], argv[2]);
	else
	{
		printf("unhandled usage\n");
	}

	FreeImage_DeInitialise();

	return 0;
}