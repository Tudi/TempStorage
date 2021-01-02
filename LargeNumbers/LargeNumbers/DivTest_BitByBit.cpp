#include "StdAfx.h"

//A*B=N=(2*a+1)*(2*b+1)
//initally (1*a+0)*(1*b+0)=N
//next, check all combinations of a and b for which the modulo is still good
class EQStore
{
public:
    EQStore() {}
    EQStore(EQStore* eq, __int64 am, __int64 bm)
    {
        //replace old a with new : ( 2*na+nm)
        amod = am * eq->coef + eq->amod;
        bmod = bm * eq->coef + eq->bmod;
        coef = eq->coef * 2;
        Mask = eq->Mask * 2 + 1;
    }
    __int64 coef, amod, bmod;
    __int64 Mask;
};

int CheckNextBitCombo(EQStore* eq, __int64 a, __int64 b, __int64 N)
{
   __int64 amod = a * eq->coef + eq->amod;
   __int64 bmod = b * eq->coef + eq->bmod;
   __int64 mod = amod * bmod;
   if ((N & eq->Mask) == (mod & eq->Mask))
       return 1;
   return 0;
}

void DivTestBitByBit_(__int64 A, __int64 B)
{
    __int64 N = A * B;
    EQStore* eq = new EQStore();
    eq->coef = 1;
    eq->amod = 0;
    eq->bmod = 0;
    eq->Mask = 1;
    std::list< EQStore*>* PrevList = new std::list< EQStore*>();
    PrevList->push_front(eq);

    for (int i = 0; i <= 20; i++)
    {
        std::list< EQStore*>* NextList = new std::list< EQStore*>();
        for (auto itr = PrevList->begin(); itr != PrevList->end(); itr++)
        {
            EQStore* eq = *itr;
#ifdef _DEBUG
            if (eq->amod == A && eq->bmod == B)
                eq = eq;
            if (eq->amod == B && eq->bmod == A)
                eq = eq;
#endif
            if (CheckNextBitCombo(eq, 0, 0, N))
            {
                if (eq->amod * eq->bmod == N)
                {
                    printf("Solution found : %lld*%lld=%lld\n", eq->amod, eq->bmod, N);
                    i = 99999999;
                    break;
                }
                EQStore* eq2 = new EQStore(eq, 0, 0);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo(eq, 0, 1, N))
            {
                EQStore* eq2 = new EQStore(eq, 0, 1);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo(eq, 1, 0, N))
            {
                EQStore* eq2 = new EQStore(eq, 1, 0);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo(eq, 1, 1, N))
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

void DivTestBitByBit()
{
    //DivTestBitByBit_(5, 7);
    DivTestBitByBit_(349, 751); // N = 262099 , SN = 511
    DivTestBitByBit_( 6871, 7673 ); // N = 52721183 , SN = 7260
    DivTestBitByBit_( 26729, 31793 ); // N = 849795097 , SN = 29151
    DivTestBitByBit_(784727, 918839); 
}
