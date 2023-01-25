#include <profile_cached.h>
#include <position_cached.h>
#include <profile_info.h>
#include <stdio.h>

void PrintTimeDate(time_t t, char* out, int maxOut)
{
    struct tm tms;

    localtime_r(&t, &tms);
    snprintf(out, maxOut, "%04d-%02d-%02d", 1900 + tms.tm_year, tms.tm_mon, tms.tm_mday);
}

void PrintProfile(const struct ProfileCached* prof, int printFlags)
{
    if (prof == NULL)
    {
        return;
    }
    if (printFlags & PI_SEPARATION_BAR)
    {
        printf("******************************************************************\n");
    }
    if (printFlags & PI_BASIC)
    {
        if (prof->fullName) printf("Name : %s\n", prof->fullName);
        if (printFlags & PI_EXTENDED)
        {
            printf("id : %d\n", prof->id);
            printf("localityId : %d\n", prof->localityId);
            printf("country Id : %d\n", (int)prof->countryId);
            if (prof->skillsHeadlineSummaryPosDescription)
            {
                printf("skillsHeadlineSummaryPosDescription : %s\n", prof->skillsHeadlineSummaryPosDescription);
            }
        }
        printf("number of : positions(%d), lastMessaged(%d), lastreplied(%d), lastpositivereply(%d), groups(%d), projects(%d), actioned(%d)\n",
            (int)kv_size(prof->positions), (int)kv_size(prof->lastMessaged), (int)kv_size(prof->lastReplied),
            (int)kv_size(prof->lastPositiveReply), (int)kv_size(prof->groups), (int)kv_size(prof->projects),
            (int)kv_size(prof->actioned));
    }
    if (printFlags & PI_POSITIONS)
    {
        printf("Positions list : \n");
        for (size_t index = 0; index < kv_size(prof->positions); index++)
        {
            struct PositionCached* pc = &kv_A(prof->positions, index);
            if (pc->title) printf("\ttitle : %s\n", pc->title);
            if (printFlags & PI_EXTENDED)
            {
                if (pc->companyName) printf("\tcompanyName : %s\n", pc->companyName);
                printf("\tcompanyId : %d\n", pc->companyId);
            }
            char start[500], end[500];
            PrintTimeDate(pc->startDate, start, sizeof(start));
            PrintTimeDate(pc->endDate, end, sizeof(end));
            printf("\tStart -> End date : %s -> %s\n", start, end);
        }
    }
    if (printFlags & PI_MESSAGED)
    {
        printf("lastMessaged list : \n");
        for (size_t index = 0; index < kv_size(prof->lastMessaged); index++)
        {
            struct Id_TimeValue* pIdTimePair = &kv_A(prof->lastMessaged, index);
            char time[500];
            PrintTimeDate(pIdTimePair->value, time, sizeof(time));
            printf("\t company %d messaged : %s \n", pIdTimePair->id, time);
        }
        printf("lastReplied list : \n");
        for (size_t index = 0; index < kv_size(prof->lastReplied); index++)
        {
            struct Id_TimeValue* pIdTimePair = &kv_A(prof->lastReplied, index);
            char time[500];
            PrintTimeDate(pIdTimePair->value, time, sizeof(time));
            printf("\t company %d replied : %s \n", pIdTimePair->id, time);
        }
        printf("lastPositiveReply list : \n");
        for (size_t index = 0; index < kv_size(prof->lastPositiveReply); index++)
        {
            struct Id_TimeValue* pIdTimePair = &kv_A(prof->lastPositiveReply, index);
            char time[500];
            PrintTimeDate(pIdTimePair->value, time, sizeof(time));
            printf("\t company %d replied : %s \n", pIdTimePair->id, time);
        }
    }
    if (printFlags & PI_GROUPS)
    {
        printf("Groups list : \n");
        for (size_t index = 0; index < kv_size(prof->groups); index++)
        {
            struct Id_Int32Value* pIdIntPair = &kv_A(prof->groups, index);
            printf("\t company %d group : %d \n", pIdIntPair->value, pIdIntPair->id);
        }
    }
    if (printFlags & PI_PROJECTS)
    {
        printf("Projects list : \n");
        for (size_t index = 0; index < kv_size(prof->projects); index++)
        {
            struct Id_Int32Value* pIdIntPair = &kv_A(prof->groups, index);
            printf("\t company %d project : %d \n", pIdIntPair->value, pIdIntPair->id);
        }
    }
    if (printFlags & PI_ACTIONED)
    {
        printf("actioned list : ");
        for (size_t index = 0; index < kv_size(prof->actioned); index++)
        {
            printf("%d,", kv_A(prof->actioned, index));
        }
        printf("\n");
    }
    if (printFlags & PI_SEPARATION_BAR)
    {
        printf("******************************************************************\n");
    }
}
