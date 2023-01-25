#include "StdAfx.h"
#include <stdio.h>
#include <ctype.h>

int encrypt_decrypt_WaterMark(const char* in_WaterMark, const char* in_encryptKey, const size_t in_KeyLen, char** out_str)
{
	*out_str = strdup2(in_WaterMark);
	if (in_KeyLen > 0)
	{
		for (size_t i = 0; in_WaterMark[i] != 0; i++)
		{
			size_t key_index = i % in_KeyLen;
			BYTE key_i = in_encryptKey[key_index];
			key_i = key_i | (1 << 7); // if key == char than result is 0. Can't be added to meta tag
			(*out_str)[i] = (*out_str)[i] ^ key_i;
		}
	}
	return 0;
}

static int addTagWaterMark(FIBITMAP* in_Img, const char* waterMarkStr, DWORD bytes_to_store)
{
	FITAG* newTag = FreeImage_CreateTag();
	if (FreeImage_SetTagKey(newTag, REVEAL_TAG_KEY) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag key");
		FreeImage_DeleteTag(newTag);
		return -1;
	}
#ifdef REVEAL_TAG_DESCRIPTION
	if (FreeImage_SetTagDescription(newTag, REVEAL_TAG_DESCRIPTION) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag description");
	}
#endif
#ifdef REVEAL_TAG_ID
	if (FreeImage_SetTagID(newTag, REVEAL_TAG_ID) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag id");
		FreeImage_DeleteTag(newTag);
		return -1;
	}
#endif
	if (FreeImage_SetTagType(newTag, FIDT_ASCII) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag type");
		FreeImage_DeleteTag(newTag);
		return -1;
	}
	if (FreeImage_SetTagCount(newTag, bytes_to_store) != TRUE) // number of items stored
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag count");
		FreeImage_DeleteTag(newTag);
		return -1;
	}
	if (FreeImage_SetTagLength(newTag, bytes_to_store) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag length");
		FreeImage_DeleteTag(newTag);
		return -1;
	}
	if (FreeImage_SetTagValue(newTag, waterMarkStr) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set tag value");
		FreeImage_DeleteTag(newTag);
		return -1;
	}
	if (FreeImage_SetMetadata(FIMD_COMMENTS, in_Img, FreeImage_GetTagKey(newTag), newTag) != TRUE)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to set metadata");
		FreeImage_DeleteTag(newTag);
		return -1;
	}

	FreeImage_DeleteTag(newTag);

	return 0;
}

static int checkImageWaterMark(FIBITMAP* in_Img, const char* waterMarkStr, size_t bytes_to_store)
{
	BitWriter bw;
	bitWriterinit(&bw, (unsigned char*)waterMarkStr, bytes_to_store * 8, 0);

	size_t pitch = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	size_t Width = FreeImage_GetWidth(in_Img);
	size_t Height = FreeImage_GetHeight(in_Img);

	if (Width * Height <= bytes_to_store * 8)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Image too small to store the watermark");
		return -1;
	}

	for (size_t row = 0; row < Height; row++)
	{
		for (size_t col = 0; col < Width; col++)
		{
			BYTE bit = bitWriterRead(&bw);
			if (bit > 1)
			{
				// all good, we have written all the bits from the message
				return 0;
			}
			BYTE bit1 = (BITS[row * pitch + col * Bytespp + 0] & 1);
			BYTE bit2 = (BITS[row * pitch + col * Bytespp + 1] & 1);
			BYTE bit3 = (BITS[row * pitch + col * Bytespp + 2] & 1);
			if (bit != bit1 || bit1 != bit2 || bit2 != bit3)
			{
				return -1;
			}
		}
	}

	return 0;
}

static int addImageWaterMark(FIBITMAP* in_Img, const char* waterMarkStr, size_t bytes_to_store)
{
	BitWriter bw;
	bitWriterinit(&bw, (unsigned char*)waterMarkStr, bytes_to_store * 8, 0);

	size_t pitch = FreeImage_GetPitch(in_Img);
	BYTE* BITS = FreeImage_GetBits(in_Img);
	size_t Width = FreeImage_GetWidth(in_Img);
	size_t Height = FreeImage_GetHeight(in_Img);

	if (Width * Height <= bytes_to_store * 8)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Image too small to store the watermark");
		return -1;
	}

	for (size_t row = 0; row < Height; row++)
	{
		for (size_t col = 0; col < Width; col++)
		{
			BYTE bit = bitWriterRead(&bw);
			if (bit > 1)
			{
				// all good, we have written all the bits from the message
				return 0;
			}
			BITS[row * pitch + col * Bytespp + 0] = (BITS[row * pitch + col * Bytespp + 0] & 0xFE) | bit;
			BITS[row * pitch + col * Bytespp + 1] = (BITS[row * pitch + col * Bytespp + 1] & 0xFE) | bit;
			BITS[row * pitch + col * Bytespp + 2] = (BITS[row * pitch + col * Bytespp + 2] & 0xFE) | bit;
		}
	}

	return 0;
}

int addWaterMark(FIBITMAP* in_Img, const char* in_WaterMark, const char* in_encryptKey, const size_t in_KeyLen)
{
	char* waterMarkStr = NULL;
	encrypt_decrypt_WaterMark(in_WaterMark, in_encryptKey, in_KeyLen, &waterMarkStr);
	DWORD bytes_to_store = (DWORD)strlen(in_WaterMark) + 1;

	addTagWaterMark(in_Img, waterMarkStr, bytes_to_store);
	addImageWaterMark(in_Img, waterMarkStr, bytes_to_store);

	free(waterMarkStr);

	return 0;
}

int checkWaterMark(FIBITMAP* in_Img, const char* in_encryptKey, const size_t in_KeyLen, char** out_WaterMark)
{
	FITAG* tagMake = NULL;
	FreeImage_GetMetadata(FIMD_COMMENTS, in_Img, REVEAL_TAG_KEY, &tagMake);
	if (tagMake == NULL)
	{
		LOG_MESSAGE(DEBUG_LOG_MSG, "Failed to extract metadata tag");
	}
	char* tagString = (char*)FreeImage_GetTagValue(tagMake);
	if (tagString == NULL)
	{
		return -1; // missing watermark tag
	}
	DWORD tagCount = FreeImage_GetTagCount(tagMake);
	if (tagCount == 0 || tagString[tagCount] != 0)
	{
		return -1; // malformed data ?
	}
	LOG_MESSAGE(DEBUG_LOG_MSG, "Tag contained string : %s", tagString);
	if (checkImageWaterMark(in_Img, tagString, tagCount) != 0)
	{
		return -1; // image deteriorated or replaced
	}
	
	encrypt_decrypt_WaterMark(tagString, in_encryptKey, in_KeyLen, out_WaterMark);

	return 0;
}
