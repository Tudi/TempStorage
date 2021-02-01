#include "StdAfx.h"

// curious if cross check would eliminate any candidates
//(b-a+k)*k+m=x(a-k)
//k*k+m=x*(a-k)					- if b==a
//k*k+m=k*(a-k)					- if x == k
//2*k*k-a*k+m=0					- moved on one side
//k12=(a+-sqrt(a*a-4*2*m))/4	- k solutions
//a^2-8*m=c^2 					- for k to be integer we are looking for an a that produces a square : c<a
//(sqrt(N)-l)^2-8*(m0+2*l*sqrt(N)-l^2)=c^2 - step based approach, l<sqrt(n), l>0, c<a-l
//ex : (30-1)^2-8*(59+43)=5^2
//ex : 8*943=9*29+5^2
//N-9*m=c^2
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

int CheckNextBitCombo3(EQStore* eq, __int64 a, __int64 b, __int64 SQN, __int64 N, __int64 m0)
{
    __int64 l = a * eq->coef + eq->amod;
    __int64 c = b * eq->coef + eq->bmod;
    //(sqrt(N)-l)^2=8*(m0+2*l*sqrt(N)-l^2)+c^2
    __int64 left1 = (SQN-l)*(SQN-l)+8*l*l;
    __int64 right1 = 8*(m0+2*l*SQN) + c * c;
    __int64 left2 = N+9*l*l;
    __int64 right2 = 9*(m0+2*l*SQN)+c*c;
    if ((left1 & eq->Mask) == (right1 & eq->Mask) && (left2 & eq->Mask) != (right2 & eq->Mask))
		printf("wow, crosscheck failed\n");
    if ((left1 & eq->Mask) != (right1 & eq->Mask) && (left2 & eq->Mask) == (right2 & eq->Mask))
		printf("wow, crosscheck failed\n");
    if ((left1 & eq->Mask) == (right1 & eq->Mask))
    {
#if 0   //obviously if you multiply it with any number, it will be true.
        //you could divide it also, but you would need to make sure that there is no information loss : non divizible k or x
        float mult = 2;
        __int64 Bb2 = (mult * Bb);
        __int64 Aa2 = (mult * Aa);
        __int64 k2 = (mult * k);
        __int64 x2 = (mult * x);
        __int64 m2 = (mult * mult * m);
        __int64 left2 = (Bb2 + k2 + x2) * k2 + m2;
        __int64 right2 = Aa2 * (x2 + k2);
        if ((left2 & eq->Mask) == (right2 & eq->Mask))
            left2 = left2;
        else
            right2 = right2;
#endif
        if (left1 == right1)
            return 3;
        return 1;
    }
    return 0;
}

void DivTestBitByBit3_(__int64 A, __int64 B)
{
    __int64 N = A * B;
    __int64 SQN = (__int64)sqrt((double)N);
    __int64 m = N - SQN * SQN;
    EQStore* eq = new EQStore();
    eq->coef = 1;
    eq->amod = 0;
    eq->bmod = 0;
    eq->Mask = 1;
    std::list< EQStore*>* PrevList = new std::list< EQStore*>();
    PrevList->push_front(eq);

//    __int64 ExpectedK = Aa - A;
//    __int64 ExpectedX = (A + B) - (Aa + Bb);
//    printf("Expecting k=%lld,x=%lld\n", ExpectedK, ExpectedX);
    CheckNextBitCombo3(eq, 1, 5, SQN, N, m);

    for (int i = 0; i <= 20; i++)
    {
        std::list< EQStore*>* NextList = new std::list< EQStore*>();
        for (auto itr = PrevList->begin(); itr != PrevList->end(); itr++)
        {
            EQStore* eq = *itr;
            if (int res = CheckNextBitCombo3(eq, 0, 0, SQN, N, m))
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
            if (CheckNextBitCombo3(eq, 0, 1, SQN, N, m))
            {
                EQStore* eq2 = new EQStore(eq, 0, 1);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo3(eq, 1, 0, SQN, N, m))
            {
                EQStore* eq2 = new EQStore(eq, 1, 0);
                NextList->push_front(eq2);
            }
            if (CheckNextBitCombo3(eq, 1, 1, SQN, N, m))
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

void DivTestBitByBit3()
{
    //DivTestBitByBit3_(5, 7);
    DivTestBitByBit3_(23, 41);
    DivTestBitByBit3_(349, 751); // N = 262099 , SN = 511
    DivTestBitByBit3_(6871, 7673); // N = 52721183 , SN = 7260
    DivTestBitByBit3_(26729, 31793); // N = 849795097 , SN = 29151
    DivTestBitByBit3_(784727, 918839);
    DivTestBitByBit3_(3, 918839);
}
