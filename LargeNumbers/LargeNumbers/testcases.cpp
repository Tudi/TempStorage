#include "StdAfx.h"

void TestSetAddMull()
{
    LargeNumber A,B,C;
	__int64 Ci;
#define TestCorrectnessLimit 2048
    for( int a = 0; a < TestCorrectnessLimit; a++ )
        for( int b = 0; b < TestCorrectnessLimit; b++ )
        {
            SetLN( A, a );
            SetLN( B, b );

            int add = a + b;
            AddLN( A, B, C );
            ToIntLN( C, Ci );
            if( add != Ci )
                printf("Add failed for %d + %d = %d != %lld\n", a, b, add, Ci );

            int mul = a * b;
            MulLN( A, B, C );
            ToIntLN( C, Ci );
            if( mul != Ci )
                printf("Add failed for %d * %d = %d != %lld\n", a, b, mul, Ci );

            int half = mul /2;
            HalfLN( C );
            ToIntLN( C, Ci );
            if( half != Ci )
                printf("half failed for %d * %d = %d != %lld\n", a, b, half, Ci );

            if( mul >= 1 )
            {
                int dec = mul - 1;
                MulLN( A, B, C );
                SubLN( C, 1 );
                ToIntLN( C, Ci );
                if( dec != Ci )
                    printf("dec failed for %d * %d = %d != %lld\n", a, b, dec, Ci );
            }
        }
}