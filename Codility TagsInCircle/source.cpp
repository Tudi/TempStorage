// you can write to stdout for debugging purposes, e.g.
// printf("this is a debug message\n");

int FindNextMinDist(int *Distances, int N)
{
    int MinDistance=0x7FFFFFFF;
    int DistanceAtLoc=-1;
    for(int i=0;i<N;i++)
        if(Distances[i]<MinDistance)
        {
            MinDistance = Distances[i];
            DistanceAtLoc = i;
        }
    return DistanceAtLoc;
}

int CanAddTag(char *TagsSoFar, int TagCount, char Tag)
{
    for(int i=0;i<TagCount;i++)
        if(TagsSoFar[i]==Tag)
            return 0;
    return 1;
}

int solution(char *S, int X[], int Y[], int N) {
    // write your code in C99 (gcc 6.2.0)
    if(N<=1)
        return N;
        
    int *Distances=(int*)malloc(N*sizeof(int));
    char *TagsIncluded=(char*)malloc(N);
    for(int i=0;i<N;i++)
        Distances[i]=X[i]*X[i]+Y[i]*Y[i];
    
    //sort array
    for(int j=0;j<N;j++)
        for(int i=j+1;i<N;i++)
            if(Distances[j]>Distances[i])
            {
                int t=Distances[j];
                Distances[j] = Distances[i];
                Distances[i] = t;
                char tc = S[j];
                S[j] = S[i];
                S[i] = tc;
            }
    int TagsAdded=1;
    TagsIncluded[0]=S[0];
    int PrevMinDistance=Distances[0];
    for(int i=1;i<N;i++)
    {
        int DistNow = Distances[i];
        if(CanAddTag(TagsIncluded,TagsAdded,S[i]) == 0)
        {
            //printf("%d-%d,%d",PrevMinDistance,Distances[i],TagsAdded);
            if(PrevMinDistance==DistNow
                && TagsAdded > 0 
                && TagsIncluded[TagsAdded-1]==S[i])
            {
                //printf("Prev tag should not be added\n");
                return TagsAdded-1;
            }
            return TagsAdded;
        }
        PrevMinDistance = DistNow;
        TagsIncluded[TagsAdded] = S[i];
        TagsAdded++;
    }
    return TagsAdded;
}
