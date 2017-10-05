#pragma once

unsigned long crc32(const unsigned char *s, unsigned int len);
int base64_encode(const void* src, const size_t len, char **p_out, int *outLen);
int base64_decode(const void* data, const size_t len, char **p_out, int *outLen);
int printStackTrace();