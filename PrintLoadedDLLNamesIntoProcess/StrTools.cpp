#include "StdAfx.h"
#include <stdarg.h>

int strpos1(const char *LongStr, const char *ShortStr)
{
    int Ind1 = 0;
    while (LongStr[Ind1] != 0)
    {
        int Ind2 = 0;
        while (LongStr[Ind1 + Ind2] == ShortStr[Ind2] && LongStr[Ind1 + Ind2] != 0)
            Ind2++;
        if (ShortStr[Ind2] == 0)
            return Ind1;
        Ind1++;
    }
    return -1;
}


void IntToStr( unsigned int Nr, char *OutBuff, int MaxLen )
{
    int WriteIndex = 0;
    if( Nr == 0 )
        OutBuff[WriteIndex++] = '0';
	  while( Nr>0 && WriteIndex < MaxLen )
    {
        OutBuff[WriteIndex++] = '0' + Nr % 10;
        Nr = Nr / 10;
    }
    //now flip the number
    for( int i=WriteIndex-1;i>=WriteIndex/2;i--)
    {
        char t = OutBuff[i];
        OutBuff[i] = OutBuff[WriteIndex-1-i];
        OutBuff[WriteIndex-1-i] = t;
    }
    //make it null terminated
    OutBuff[WriteIndex++] = 0;
}

void ConcatStr( char *out_s1, int MaxLenS1, char *in_s2 )
{
    if(out_s1==NULL)
        return;
    if(in_s2==NULL)
        return;
    int i,j;
    for( i=0;i<MaxLenS1 && out_s1[i]!=0;i++ );
    for( j = 0;i+j<MaxLenS1 && in_s2[j]!=0;j++)
        out_s1[i+j] = in_s2[j];
    if(i+j<MaxLenS1)
        out_s1[i+j]=0;
    else
        out_s1[MaxLenS1-1]=0;
}

void Concat( char *out_s1, int MaxLenS1, int in_nr )
{
    char TempBuff[20];
    IntToStr( in_nr, TempBuff, sizeof( TempBuff ) );
    ConcatStr( out_s1, MaxLenS1, TempBuff );
}

void StrCpy( char *OutStr, int MaxLen, const char *InStr )
{
    for( int i = 0; i < MaxLen && InStr[i] != 0; i++ )
        OutStr[i] = InStr[i];
    if( i < MaxLen )
        OutStr[i] = 0;
    else
        OutStr[MaxLen-1] = 0;
}

void StrSimplePrint( char *OutStr, int MaxLen, const char *fmt, ... )
{
    int WriteIndex = 0;
    va_list ap;
    va_start(ap, fmt);
    //parse the string and guess the type of the next argument
    for( int i=0;i<MaxLen && WriteIndex < MaxLen && fmt[i] != 0; i++ )
    {
        int TokenFound = 0;
        if( fmt[i] == '%' )
        {
            if( fmt[i+1] == 'd' )
            {
                int val = va_arg(ap, int);
                OutStr[WriteIndex]=0;
                Concat( &OutStr[WriteIndex], MaxLen - WriteIndex, val );
                i++;
                TokenFound = 1;
            }
            else if( fmt[i+1] == 's' )
            {
                char *val = va_arg(ap, char*);
                OutStr[WriteIndex]=0;
                ConcatStr( &OutStr[WriteIndex], MaxLen - WriteIndex, val );
                i++;
                TokenFound = 1;
            }
        }
        if( TokenFound == 0 )
            OutStr[WriteIndex++]=fmt[i];
        else
            for( ; OutStr[WriteIndex] != 0 && WriteIndex < MaxLen; WriteIndex++ );
    }
    va_end(ap);

    if( WriteIndex < MaxLen )
        OutStr[WriteIndex]=0;
    else
        OutStr[MaxLen-1]=0;
}