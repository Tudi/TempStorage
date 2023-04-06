#include <stdio.h>
#include "openssl_dec.h"
#include "data_loader.h"

int main() 
{
    LoadFileContent();
    ResetFileReadStatus();

    unsigned char* data1, * data2;
    GetDataBlocks(&data1, &data2);

    unsigned char key1[255], key2[255], key3[255], key4[255];
    int key1Len = 16, key2Len = 16, key3Len = 16, key4Len = 16;
    int decRet = 1;
    while (GetNextKeyBuffs(key1, key2, key3, key4) == 0)
    {
//        GetDataBlocks(&data1, &data2);
        decRet = decrypt_with(AES_128,
            data1, 8, key1, key1Len, key2, key2Len, 
            data2, 8, key3, key3Len, key4, key4Len);
        if (decRet == 1)
        {
            printf("Decrypt match\n");
        }
    }
    return 0;
}
