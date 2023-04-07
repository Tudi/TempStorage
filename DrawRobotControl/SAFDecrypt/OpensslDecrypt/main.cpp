#include <stdio.h>
#include "openssl_dec.h"
#include "data_loader.h"

void TryAlg_AES(unsigned char* data1, unsigned char* data2)
{
    unsigned char key1[255], key2[255], key3[255], key4[255];
    int key1Len = 16, key2Len = 16, key3Len = 16, key4Len = 16;
    int decRet = 1;
    int tryKeySizes[] = { 16, 24, 32, 0 };
    printf("Trying algorithm AES-CBC\n");
    for (int keyind = 1; tryKeySizes[keyind] != 0; keyind++)
    {
        printf("Trying key size %d\n", tryKeySizes[keyind]);
        key1Len = tryKeySizes[keyind];
        ResetFileReadStatus();
        SetKeySizes(key1Len, key2Len);
        while (AES_GetNextKeyBuffs(key1, key2, key3, key4) == 0)
        {
            //        GetDataBlocks(&data1, &data2);
            decRet = decrypt_with(DEC_AES_CBC,
                data1, 16, key1, key1Len, key2, key2Len,
                data2, 16, key3, key3Len, key4, key4Len);
            if (decRet == 1)
            {
                printf("Decrypt match for AES-CBC. Keylen %d\n", key1Len);
            }
        }
    }
}

void TryAlg_RSA(unsigned char* data1, unsigned char* data2)
{
    printf("Trying algorithm RSA\n");
    int tryKeySizes[] = { 16, 24, 32, 0 };
    unsigned char key1[255], key2[255], key3[255], key4[255];
    int key1Len = 16, key2Len = 16, key3Len = 16, key4Len = 16;
    for (int keyind = 0; tryKeySizes[keyind] != 0; keyind++)
    {
        printf("Trying key size %d\n", tryKeySizes[keyind]);
        int key1Len = tryKeySizes[keyind];
        ResetFileReadStatus();
        SetKeySizes(key1Len, 0);
        while (RSA_GetNextKeyBuffs(key1, key2) == 0)
        {
            //        GetDataBlocks(&data1, &data2);
            int decRet = decrypt_with(DEC_RSA,
                data1, 16, key1, key1Len, key2, key2Len,
                data2, 16, key3, key3Len, key4, key4Len);
            if (decRet == 1)
            {
                printf("Decrypt match for RSA. Keylen %d\n", key1Len);
            }
        }
    }
}

int main() 
{
    LoadFileContent();
    ResetFileReadStatus();

    unsigned char* data1, * data2;
    GetDataBlocks(&data1, &data2);

//    TryAlg_AES(data1, data2);
    TryAlg_RSA(data1, data2);

    return 0;
}
