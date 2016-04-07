#pragma once

//this file exists only because unloading baseCRT will take away a lot of string functions

int strpos1(const char *LongStr, const char *ShortStr);
void IntToStr( unsigned int Nr, char *OutBuff, int MaxLen );
void ConcatStr( char *out_s1, int MaxLenS1, char *in_s2 );
void Concat( char *out_s1, int MaxLenS1, int in_nr );
void StrCpy( char *OutStr, int MaxLen, const char *InStr );
void StrSimplePrint( char *OutStr, int MaxLen, const char *fmt, ... );


//!! this expects wide char type pointer
template<typename T>
void WideCharStr2Str1(T *pwstr, char *str, int maxlen)
{
    char *wstr = (char*)pwstr;
    if (str == NULL)
        return;
    str[0] = 0;
    if (wstr == NULL)
        return;
    //luck based check to see this string is indeed wide char.
    if (wstr[0] != 0 && wstr[1] != 0)
    {
        strcpy(str, (char*)wstr);
        return;
    }
    int i;
    for (i = 0; i < maxlen * 2 && wstr[i] != 0; i += 2)
        str[i / 2] = (char)wstr[i];
    if (i / 2<maxlen)
        str[i / 2] = 0;
}