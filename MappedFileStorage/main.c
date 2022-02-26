#include "profile_persistent2.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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

int main()
{
	printf("Construct structure\n");
	GenericSerializedStructure* prof1 = createGenericSerializableStruct(PPFN_MAX_FIELD_NAME, PROFILE_PERSISTENT_WRITER_VERSION);
	appendGenericSerializableStructDataSafe(&prof1, PPFN_FIRST_NAME, SFT_NULL_STRING, "first name", strlen("first name") + 1);
	appendGenericSerializableStructDataSafe(&prof1, PPFN_LAST_NAME, SFT_NULL_STRING, "last name", strlen("last name") + 1);

	printf("Serialize structure\n");
	writeDataToFile(prof1);
	free(prof1);

	printf("De serialize structure\n");
	prof1 = readDataFromFile();

	printf("Check data content :\n");
	char* tStr;
	int tstrSize;
	getGenericSerializableStructDataSafe(prof1, PPFN_FIRST_NAME, SFT_NULL_STRING, &tStr, &tstrSize);
	printf("\tfirst name : %s\n", tStr);
	getGenericSerializableStructDataSafe(prof1, PPFN_LAST_NAME, SFT_NULL_STRING, &tStr, &tstrSize);
	printf("\tlast name : %s\n", tStr);

	printf("all done\n");
	return 0;
}