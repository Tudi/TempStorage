#pragma once

int LoadFileContent();
void ResetFileReadStatus();
int GetDataBlocks(unsigned char** out_buf1, unsigned char** out_buf2);
int AES_GetNextKeyBuffs(unsigned char* out1_buf, unsigned char* out2_buf, unsigned char* out3_buf, unsigned char* out4_buf);
int RSA_GetNextKeyBuffs(unsigned char* out1_buf, unsigned char* out2_buf);
void SetKeySizes(int k1, int k2);