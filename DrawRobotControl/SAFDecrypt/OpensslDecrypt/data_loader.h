#pragma once

int LoadFileContent();
void ResetFileReadStatus();
int GetDataBlocks(unsigned char** out_buf1, unsigned char** out_buf2);
int GetNextKeyBuffs(unsigned char* out1_buf, unsigned char* out2_buf, unsigned char* out3_buf, unsigned char* out4_buf);
