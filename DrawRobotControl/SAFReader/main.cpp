#include "stdafx.h"

/*
void TryDecryptDB3()
{
	byte iv1[CryptoPP::AES::BLOCKSIZE];
	byte iv2[CryptoPP::AES::BLOCKSIZE];
	byte hash1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash2[CryptoPP::SHA256::DIGESTSIZE];

	const int dataSize = 176;
	byte data1[dataSize];
	byte data2[dataSize];
	byte data_res1[dataSize];
	byte data_res2[dataSize];
	byte hash_res1[CryptoPP::SHA256::DIGESTSIZE];
	byte hash_res2[CryptoPP::SHA256::DIGESTSIZE];

	LoadIV_Data_Hash_From("1.saf", iv1, data1, dataSize, hash1);
	LoadIV_Data_Hash_From("2.saf", iv2, data2, dataSize, hash2);

	Decrypt1Block_CBC_AES_256_NOAUTH(iv1, key, data1, dataSize, data_res1);
	Decrypt1Block_CBC_AES_256_NOAUTH(iv2, key, data2, dataSize, data_res2);

	if (memcmp(data_res1, data_res2, dataSize) == 0)
	{
		GetDataBlockHash(iv1, data_res1, dataSize, hash_res1);
		GetDataBlockHash(iv2, data_res2, dataSize, hash_res2);
	}
}
*/

int main(int argc, char **argv)
{
	SAFFile SAFReader;

	if (argc > 1)
	{
		printf("Reading file content : %s\n", argv[1]);
		SAFReader.ReadFile(argv[1]);
	}
	else
	{
//		SAFReader.ReadFile("1.saf");
		//SAFReader.ReadFile("0006 Three Vertical Two Inch Lines From -1 to 1 Two Inches Apart.saf");
		//SAFReader.ReadFile("0008 Vertical Half Inch Line followed by Horizontal Half Inch Line without Transition.saf");
		//SAFReader.ReadFile("0007 Vertical Half Inch Line followed by Horizontal Half Inch Line with Transition.saf");
		//SAFReader.ReadFile("0012 Half Inch Lines Angles beginning from top_30_60_90_120_150_180_21_240_270_300_330.saf");
//		SAFReader.ReadFile("0018 Names From Excel in Block.saf");
		SAFReader.ReadFile("two lines with transition.saf");
	}

	SAFReader.PrintContent();

#if 0
	FILE* f;
//	errno_t openErr = fopen_s(&f, "1.saf", "rb");
	if (f)
	{
		SAF_File_Info db1;
		GetDB1Data(f, &db1);
		SAF_File_Info2 db2;
		GetDB2Data(f, &db2);

		int readOffset = SAF_DB3_OFSET;

		SAF_TransitionInfo db3;
		DB4Layout db4;
		for (size_t i = 0; i < db1.transitionCount1; i++)
		{
			GetDB3Data(f, readOffset , &db3);
			GetDB4Data(f, readOffset, db3.lineCount, db3.pointCount, &db4);
		}
		fclose(f);
	}
#endif
}