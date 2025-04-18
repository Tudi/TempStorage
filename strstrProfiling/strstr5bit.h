#ifndef _STRSTR_5_BIT_H_
#define _STRSTR_5_BIT_H_

typedef struct str5Bit {
	unsigned short len; // this is character len. byte len is (len*5+7)/8
	char* str; // this is not a null terminated string !
}str5Bit;

/// <summary>
/// Initialize a map that will translate 8 bit chars to our reduced size dictionary
/// </summary>
void InitConversionMap5Bit();

/// <summary>
/// So that it could be used as a static var
/// </summary>
void PrintConversionMap5Bit();

// constructor / destructor
void initStr5Bit(str5Bit* str);
void freeStr5Bit(str5Bit* str);

/// <summary>
/// Simple ascii text lower case only uses 36 character : 10 numbers + 26 chars + 33 punctuations
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
str5Bit* ConvertTo5Bit(const char* str, str5Bit* out_dst);

/// <summary>
/// Convert a 5bit str to ASCII char str
/// </summary>
/// <param name="str5bit"></param>
/// <returns></returns>
char* ConvertFrom5Bit(const str5Bit* str5bit);

/// <summary>
/// In a 5 bit str, check if the substring exists
/// Specially made for SMALL !! strings ( less than 20 bytes )
/// </summary>
/// <param name="largeStr"></param>
/// <param name="subStr"></param>
/// <returns></returns>
size_t HasStr5Bit(const str5Bit* largeStr, const str5Bit* subStr);

void RunDebug5BitTests();

/// <summary>
/// Just how much worse is this compared to normal strstr. It does save memory....
/// </summary>
void Run_strstr_5Bit();

#endif