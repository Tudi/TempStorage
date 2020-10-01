#include <string.h>
#include <stdio.h>

int CountAs(char* S, int Start, int End)
{
    int ret = 0;
    for (int i = Start; i < End; i++)
        if (S[i] == 'a')
            ret++;
    return ret;
}

int main()
{
    char* S = (char*)"ababa";
    // write your code in C99 (gcc 6.2.0)
    int LenS = strlen(S);
    if (LenS < 3)
        return 0;
    //printf("strlen=%d\n", LenS);
    int Lengths[3] = { 1, 1, 0 };
    int ret = 0;
    while (Lengths[2] == 0)
    {
        //printf("l1=%d,l2=%d,l3=%d\n", Lengths[0], Lengths[1], LenS - (Lengths[0] + Lengths[1]));
        int c1 = CountAs(S, 0, Lengths[0]);
        int c2 = CountAs(S, Lengths[0], Lengths[0] + Lengths[1]);
        int c3 = CountAs(S, Lengths[0] + Lengths[1], LenS);
        if (c1 == c2 && c2 == c3)
            ret++;
        //next combo
        int BadCombo = 1;
        do {
            Lengths[0] += 1;
            //normalize
            for (int i = 0; i < 2; i++)
                if (Lengths[i] > LenS - 1)
                {
                    Lengths[i] = 1;
                    Lengths[i + 1]++;
                }
            if (Lengths[0] + Lengths[1] <= LenS - 1)
                BadCombo = 0;
        } while (BadCombo == 1 && Lengths[2] == 0);
    }
    printf("%d\n", ret);
    return ret;
}