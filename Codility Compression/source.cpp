// you can write to stdout for debugging purposes, e.g.
// printf("this is a debug message\n");

int DigitCount(int N)
{
    if(N==0)
        return 1;
    int count=0;
    while(N>0)
    {
        count++;
        N=N/10;
    }
    return count;
}

int GetCompressedLen(int *RepeatCount, int Count)
{
    int Len=Count;
    for(int i=0;i<Count;i++)
    {
        if(RepeatCount[i]>1)
            Len += DigitCount(RepeatCount[i]);
    }
    return Len;
}

int GenCompressedWithMask(char *S, int N, int AtPos, int K,int *RepeatCount)
{
    char PrevChar=0;
    int  CurCharCounter=1;
    int SegmentId=0;
    for(int i=0;i<N;i++)
    {
        if(i>=AtPos && i<AtPos+K)
            continue;
        //printf("%c",S[i]);
        if(S[i]==PrevChar)
        {
            CurCharCounter++;
        }
        else
        {
            if(PrevChar!=0)
            {
                RepeatCount[SegmentId]=CurCharCounter;
                SegmentId++;
                CurCharCounter=1;
            }
            PrevChar=S[i];
        }
    }
    //printf("\n");
    //write last char also
    RepeatCount[SegmentId]=CurCharCounter;
    SegmentId++;
    return SegmentId;
}

int solution(char *S, int K) {
    // write your code in C99 (gcc 6.2.0)
    int N;
    N = strlen(S);
    if(N==0)
        return 0;
    int *RepeatCount=(int*)malloc(N*sizeof(int));

    int BestGains=N;
    for(int i=0;i<N-K;i++)
    {
        int DistinctCount = GenCompressedWithMask(S,N,i,K,RepeatCount);
        //printf("%d=",i);
        //for(int i=0;i<DistinctCount;i++)
        //    printf("%d,",RepeatCount[i]);
        //printf("\n");
        int CurLen = GetCompressedLen(RepeatCount,DistinctCount);
        //printf("Curlen=%d\n",CurLen);
        if(CurLen<BestGains)
        {
            BestGains = CurLen;
        }
    }
    //printf("K=%d\n",K);
    return BestGains;
}
