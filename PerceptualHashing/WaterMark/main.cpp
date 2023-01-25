#include "StdAfx.h"

enum AppRequestTypes
{
	APP_REQ_ENCRYPT_STRING = 1,
	APP_REQ_EXTRACT_WATERMARK,
	APP_REQ_BURN_WATERMARK,
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

int waterMarkImage(const char *fileName)
{
	FIBITMAP* dib = openImage(fileName);

	const char* encryptKey = "1234123";
	char* outWaterMark = NULL;
	checkWaterMark(dib, encryptKey, strlen(encryptKey), &outWaterMark);
	addWaterMark(dib,"burn this string", encryptKey, strlen(encryptKey));

	// free the dib
	FreeImage_Unload(dib);

	return 0;
}

void printHelp(const char *appName, const char *printReason)
{
	if (printReason != NULL)
	{
		printf("%s\n", printReason);
	}
	printf("Usage 1: Encrypt/ Decrypt string : %s 1 WaterMarkString EncryptorString\n", appName);
	printf("Usage 2: Extract WaterMark : %s 2 SourceFile DecryptorString\n", appName);
	printf("Usage 3: Burn WaterMark : %s 3 SourceFile DestFile WaterMarkString DecryptorString\n", appName);
}

static int handleReqEncDec(int argc, char** argv)
{
	if (argc < 4)
	{
		printHelp(argv[0], "Not enough params to encrypt string");
		return 1;
	}
	const char* strToEncrypt = argv[2];
	const char* strEncryptor = argv[3];
	if (strlen(strToEncrypt) == 0)
	{
		printHelp(argv[0], "Encrypted string can't be empty");
		return 1;
	}
	if (strlen(strEncryptor) == 0)
	{
		printHelp(argv[0], "Encrypt string can't be empty");
		return 1;
	}
	char* encryptedString = NULL;
	int res = encrypt_decrypt_WaterMark(strToEncrypt, strEncryptor, strlen(strEncryptor), &encryptedString);
	if (res == 0 && encryptedString != NULL)
	{
		printf("%s", encryptedString);
	}
	free(encryptedString);

	return 0;
}

static int handleReqExtractWaterMark(int argc, char** argv)
{
	if (argc < 3)
	{
		printHelp(argv[0], "Not enough params to extract watermark");
		return 1;
	}
	const char* strFileName = argv[2];
	const char* strEncryptor = NULL;
	size_t decryptLen = 0;
	if (argc >= 3)
	{
		strEncryptor = argv[3];
		decryptLen = strlen(strEncryptor);
	}
	if (strlen(strFileName) == 0)
	{
		printHelp(argv[0], "FileName can't be empty");
		return 1;
	}

	FIBITMAP* dib = openImage(strFileName);

	if (dib == NULL)
	{
		printHelp(argv[0], "Could not open Input file");
		return 1;
	}

	char* watermark = NULL;
	int ret = checkWaterMark(dib, strEncryptor, decryptLen, &watermark);
	if (ret == 0 && watermark != NULL)
	{
		printf("%s", watermark);
	}
	free(watermark);

	FreeImage_Unload(dib);

	return 0;
}

static int handleReqBurnWaterMark(int argc, char** argv)
{
	if (argc < 6)
	{
		printHelp(argv[0], "Not enough params to burn watermark");
		return 1;
	}
	const char* strInFileName = argv[2];
	const char* strOutFileName = argv[3];
	const char* strWatermark = argv[4];
	const char* strEncryptor = argv[5];
	size_t decryptLen = strlen(strEncryptor);
	if (strlen(strInFileName) == 0)
	{
		printHelp(argv[0], "Input FileName can't be empty");
		return 1;
	}
	if (strlen(strOutFileName) == 0)
	{
		printHelp(argv[0], "Output FileName can't be empty");
		return 1;
	}

	FIBITMAP* dib = openImage(strInFileName);

	if (dib == NULL)
	{
		printHelp(argv[0], "Could not open Input file");
		return 1;
	}

	addWaterMark(dib, strWatermark, strEncryptor, decryptLen);

	if (FreeImage_Save(FIF_PNG, dib, strOutFileName, PNG_Z_DEFAULT_COMPRESSION) != TRUE)
	{
		printf("Failed to save image to %s\n", strOutFileName);
	}

	FreeImage_Unload(dib);

	return 0;
}

int main(int argc, char **argv)
{
#ifdef _DEBUG
	logger_setLogLevel(DEBUG_LOG_MSG);
	char* argv3[] = { (char*)".exe", (char*)"3", (char*)"../../page_fetcher/images/cryptopunks/8314.png", (char*)"d:\\GitHub\\Rev3al\\watermark\\WaterMark\\t.png", (char*)"WaterMarkSring", (char*)"EncryptorString" };
	handleReqBurnWaterMark(6, argv3); 
	char* argv2[] = { (char*)".exe", (char*)"2", (char*)"d:\\GitHub\\Rev3al\\watermark\\WaterMark\\t.png", (char*)"EncryptorString" };
	handleReqExtractWaterMark(3, argv2);
	return 0;
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
	if (appReqType == APP_REQ_ENCRYPT_STRING)
	{
		handleReqEncDec(argc, argv);
	}
	else if (appReqType == APP_REQ_EXTRACT_WATERMARK)
	{
		handleReqExtractWaterMark(argc, argv);
	}
	else if (appReqType == APP_REQ_BURN_WATERMARK)
	{
		handleReqBurnWaterMark(argc, argv);
	}

	FreeImage_DeInitialise();

	return 0;
}