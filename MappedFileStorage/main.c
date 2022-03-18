#include "profile_persistent_schema.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#define FILE_NAME "db.raw"

void writeDataToFile(GenericSerializedStructure* data)
{
	FILE* f = fopen(FILE_NAME, "wb");
	fwrite(data, data->size, 1, f);
	fclose(f);
}

GenericSerializedStructure* readDataFromFile()
{
	FILE* f = fopen(FILE_NAME, "rb");
	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);  

	GenericSerializedStructure* ret = malloc(fsize + 1);
	fread(ret, fsize, 1, f);
	fclose(f);

	return ret;
}

uint8_t* int32ToBinary(uint8_t* byteStream, int32_t i)
{
	memcpy(byteStream, &i, sizeof(i));
	return byteStream + sizeof(i);
}

const uint8_t* binaryToInt32(const uint8_t* byteStream, int32_t* i)
{
	memcpy(i, byteStream, sizeof(*i));
	return byteStream + sizeof(*i);
}


int main()
{
	printf("Construct structure\n");
	unsigned int bytesAllocatedProf1 = 0;
	GenericSerializedStructure* prof1 = create_GSS(PPFN_MAX_FIELD_NAME, &bytesAllocatedProf1);
	setData_GSS(&prof1, &bytesAllocatedProf1, PPFN_FIRST_NAME, DFID_PROFILE_FIRST_NAME, "first name", strlen("first name") + 1);
	setData_GSS(&prof1, &bytesAllocatedProf1, PPFN_LAST_NAME, DFID_PROFILE_LAST_NAME, "last name", strlen("last name") + 1);
	int profileId = 2;
	serializeBackWardCompatible(prof1, bytesAllocatedProf1, PPFN_ID, DFID_PROFILE_ID, int32ToBinary, profileId);

	printf("Serialize structure\n");
	UpdateCRC_GSS(prof1);
	writeDataToFile(prof1);
	free(prof1);

	printf("De serialize structure\n");
	prof1 = readDataFromFile();
	if (CheckCRC_GSS(prof1) != 0)
	{
		printf("CRC check failed on loaded data!\n");
	}

	printf("Check data content :\n");
	char* tStr;
	getData_GSS(prof1, PPFN_FIRST_NAME, DFID_PROFILE_FIRST_NAME, &tStr);
	printf("\tfirst name : %s\n", tStr);
	getData_GSS(prof1, PPFN_LAST_NAME, DFID_PROFILE_LAST_NAME, &tStr);
	printf("\tlast name : %s\n", tStr);
	deSerializeBackWardCompatible(prof1, PPFN_ID, DFID_PROFILE_ID, binaryToInt32, profileId);
	printf("\tprofile id : %d\n", profileId);

	printf("all done\n");
	return 0;
}