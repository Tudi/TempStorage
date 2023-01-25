#include "StdAfx.h"
enum AppRequestTypes
{
	APP_REQ_GET_HASHES = 1,
	APP_REQ_COMPARE_IMAGE,
	APP_REQ_MAX,
};

FIBITMAP* openImage(const char* fileName)
{
	FIBITMAP* dib = NULL;
	// open and load the file using the default load option
	const char* InputFileName = fileName;
	if (InputFileName == NULL)
	{
		return NULL;
	}
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

void printHelp(const char* appName, const char* printReason)
{
	if (printReason != NULL)
	{
		printf("%s\n", printReason);
	}
	printf("Usage 1: get image Hashes : %s 1 InputPath OriginalFileName\n", appName);
	printf("Usage 2: compare 2 images : %s 2 img1 img2\n", appName);
}

int printHashesOf1Image(const char *fileName, const char *realName)
{
	FIBITMAP* dib = openImage(fileName);

	if (dib == NULL)
	{
		return -1;
	}

	A_HASH_RGB aHash, dHash;
	aHashInit(&aHash, 64);
	aHashInit(&dHash, 64);
	genADHashGrayScale(dib, &aHash, &dHash);

	A_HASH_RGB pHash;
	aHashInit(&pHash, 64);
	genPHashGrayScale(dib, &pHash);

	A_HASH_RGB mHash1;
	aHashInit(&mHash1, 64);
	genMHash(fileName, realName, dib, &mHash1);

	printf("{\"AHASH\":\"");
	printAHash(&aHash, 1);
	printf("\",\"DHASH\":\"");
	printAHash(&dHash, 1);
	printf("\",\"PHASH\":\"");
	printAHash(&pHash, 1);
	printf("\",\"MHASH\":\"");
	printAHash(&mHash1, 1);
	printf("\"}");

	// free the dib
	FreeImage_Unload(dib);

	return 0;
}

int printImageSimilarityScore(const char* fileName1, const char* fileName2)
{
	FIBITMAP* dib1 = openImage(fileName1);
	FIBITMAP* dib2 = openImage(fileName2);

	A_HASH_RGB aHash1, dHash1;
	aHashInit(&aHash1, 64);
	aHashInit(&dHash1, 64);
	genADHashGrayScale(dib1, &aHash1, &dHash1);
	A_HASH_RGB pHash1;
	aHashInit(&pHash1, 64);
	genPHashGrayScale(dib1, &pHash1);

	A_HASH_RGB aHash2, dHash2;
	aHashInit(&aHash2, 64);
	aHashInit(&dHash2, 64);
	genADHashGrayScale(dib2, &aHash2, &dHash2);
	A_HASH_RGB pHash2;
	aHashInit(&pHash2, 64);
	genPHashGrayScale(dib2, &pHash2);

	A_HASH_RGB_CompareResult aHash, dHash, pHash;
	compareHash(&aHash1, &aHash2, &aHash);
	compareHash(&dHash1, &dHash2, &dHash);
	compareHash(&pHash1, &pHash2, &pHash);

	double overallMatchChance = MAX(aHash.pctMatchAvg, pHash.pctMatchAvg);
	printf("%f match chance\n", overallMatchChance);
	printf("%f A match chance\n", aHash.pctMatchAvg);
	printf("%f D match chance\n", dHash.pctMatchAvg);
	printf("%f P match chance\n", pHash.pctMatchAvg);

	// free the dib
	FreeImage_Unload(dib1);
	FreeImage_Unload(dib2);

	return 0;

}

int main(int argc, char **argv)
{
#ifdef _DEBUG
	//	printImageSimilarityScore("../../page_fetcher/images/cryptopunks/8300.png", "../../image_manip/resize/8300.png");
	//	printImageSimilarityScore("../../page_fetcher/images/cryptopunks/8300.png", "../../image_manip/blurr/8300.png");
	return printHashesOf1Image("./8314.png", "8314.png");
#endif
	logger_setLogLevel(INFO_LOG_MSG);

	if (argc == 1)
	{
		printHelp(argv[0], NULL);
		return 1;
	}
	int appReqType = 0;
	if (argc > 2)
	{
		appReqType = atoi(argv[1]);
	}
	if (appReqType <= 0 || appReqType >= APP_REQ_MAX)
	{
		printHelp(argv[0], "Invalid request type");
		return 1;
	}

	if (appReqType == APP_REQ_GET_HASHES)
	{
		printHashesOf1Image(argv[2], argv[3]);
	}
	else if (appReqType == APP_REQ_COMPARE_IMAGE)
	{
		printImageSimilarityScore(argv[2], argv[3]);
	}

	FreeImage_DeInitialise();

	return 0;
}