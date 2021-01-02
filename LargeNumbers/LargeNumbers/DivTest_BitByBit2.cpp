#include "StdAfx.h"

// just curious if this produces exactly same amount of combinations as a simple a*b=N
//(b-a+k)*k+m=x(a-k)
//(b+k+x)*k+m=a*(x+k)
class EQStore
{
public:
    EQStore() {}
    EQStore(EQStore* eq, __int64 a, __int64 b)
    {
        //replace old a with new : ( 2*na+nm)
        amod = a * eq->coef + eq->amod;
        bmod = b * eq->coef + eq->bmod;
        coef = eq->coef * 2;
        Mask = eq->Mask * 2 + 1;
    }
    __int64 coef, amod, bmod;
    __int64 Mask;
};

int CheckNextBitCombo(EQStore* eq, __int64 a, __int64 b, __int64 Aa, __int64 Bb, __int64 m)
{
    __int64 k = a * eq->coef + eq->amod;
    __int64 x = b * eq->coef + eq->bmod;
    //(b+k+x)*k+m=a*(x+k)
    __int64 left = (Bb+k+x)*k+m;
    __int64 right = Aa*(x+k);
    if ((left & eq->Mask) == (right & eq->Mask))
    {
        if (left == right)
            return 3;
        return 1;
    }
    return 0;
}

void DivTestBitByBit2_(__int64 A, __int64 B)
{
    __int64 N = A * B;
    __int64 Aa = sqrt(N);
//    __int64 Aa = A + (sqrt(N) - A) / 2;
    __int64 Bb = N / Aa;
    __int64 m = N - Aa * Bb;
    __int64 ExpectedK = Aa - A;
    __int64 ExpectedX = (A + B) - (Aa + Bb);
    printf("Expecting k=%lld,x=%lld\n", ExpectedK, ExpectedX);
    EQStore* eq = new EQStore();
    eq->coef = 1;
    eq->amod = 0;
    eq->bmod = 0;
    eq->Mask = 1;
    std::list< EQStore*>* PrevList = new std::list< EQStore*>();
    PrevList->push_front(eq);

    CheckNextBitCombo(eq, 162, 77, Aa, Bb, m);

    for (int i = 0; i <= 20; i++)
    {
        std::list< EQStore*>* NextList = new std::list< EQStore*>();
        for (auto itr = PrevList->begin(); itr != PrevList->end(); itr++)
        {
            EQStore* eq = *itr;
#ifdef _DEBUG
            if (eq->amod == ExpectedK && eq->bmod == ExpectedX)
                eq = eq;
            if (eq->amod == ExpectedX && eq->bmod == ExpectedK)
                eq = eq;
#endif
            if (int res = CheckNextBitCombo(eq, 0, 0, Aa, Bb, m))
            {
                if (res == 3)
                {
                    printf("Solution found : %lld*%lld=%lld\n", eq->amod, eq->bmod, N);
                    i = 99999999;
                    break;
                }
                EQStore* eq2 = new EQStore(eq, 0, 0);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo(eq, 0, 1, Aa, Bb, m))
            {
                EQStore* eq2 = new EQStore(eq, 0, 1);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo(eq, 1, 0, Aa, Bb, m))
            {
                EQStore* eq2 = new EQStore(eq, 1, 0);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo(eq, 1, 1, Aa, Bb, m))
            {
                EQStore* eq2 = new EQStore(eq, 1, 1);
                NextList->push_front(eq2);
            }
            delete eq;
        }
        PrevList->clear();
        delete PrevList;
        PrevList = NextList;
#ifdef _DEBUG
        printf("NextList size = %d\n", (int)NextList->size());
#endif
    }
}

void DivTestBitByBit2()
{
    //DivTestBitByBit_(5, 7);
    DivTestBitByBit2_(349, 751); // N = 262099 , SN = 511
    DivTestBitByBit2_(6871, 7673); // N = 52721183 , SN = 7260
    DivTestBitByBit2_(26729, 31793); // N = 849795097 , SN = 29151
    DivTestBitByBit2_(784727, 918839);
    DivTestBitByBit2_(3, 918839);
}
