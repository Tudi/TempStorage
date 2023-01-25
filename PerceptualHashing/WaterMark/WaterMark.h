#pragma once

struct FIBITMAP;

#define REVEAL_TAG_KEY "Rev3alIdKey"
//#define REVEAL_TAG_DESCRIPTION "Unique ID to identify an image and it's owner" // not written to file
#define REVEAL_TAG_ID (WORD)20220904

int encrypt_decrypt_WaterMark(const char* in_WaterMark, const char* in_encryptKey, const size_t in_KeyLen, char** out_str);
int checkWaterMark(FIBITMAP* in_Img, const char* in_encryptKey, const size_t in_KeyLen, char **out_WaterMark);
int addWaterMark(FIBITMAP* in_Img, const char* in_WaterMark, const char* in_encryptKey, const size_t in_KeyLen);