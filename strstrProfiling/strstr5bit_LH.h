#ifndef _STRSTR_5_BIT_LH_H_
#define _STRSTR_5_BIT_LH_H_

// The string is 'inplace' to avoid using pointer redirection
typedef struct str5BitLH {
	unsigned short len; // this is character len. byte len is (len*5+7)/8
	unsigned char str[0]; // low 4 bits followed by high 1 bit
}str5BitLH;

/// <summary>
/// Simple ascii text lower case only uses 36 character : 10 numbers + 26 chars + 33 punctuations
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
str5BitLH* ConvertTo5BitLH(const char* str, str5BitLH* out_dst, size_t bufSize);

/// <summary>
/// In a 5 bit str, check if the substring exists
/// Specially made for SMALL !! strings ( less than 20 bytes )
/// </summary>
/// <param name="largeStr"></param>
/// <param name="subStr"></param>
/// <returns></returns>
size_t HasStr5BitLH(const str5BitLH* largeStr, const str5BitLH* subStr);

/// <summary>
/// Just how much worse is this compared to normal strstr. It does save memory....
/// </summary>
void Run_strstr_5Bit_LH();

#endif