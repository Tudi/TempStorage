#include <profile_cached.h>
#include <position_cached.h>
#include <filters.h>
#include <strings_ext.h>
#include "filter_info.h"
#include <stdio.h>

void PrintFilter(struct PerformFilter* filter, int printFlags)
{
    if (filter == NULL)
    {
        return;
    }
    if (printFlags & FI_SEPARATION_BAR)
    {
        printf("******************************************************************\n");
    }
    if (printFlags & FI_BASIC)
    {
        printf("number of : filters(%d)\n", (int)kv_size(filter->filters));
    }
    if (printFlags & FI_FILTER_TEXTS)
    {
        printf("filters list : \n");
        for (size_t index = 0; index < kv_size(filter->filters); index++)
        {
            struct CompanyRoleFilter* crf = &kv_A(filter->filters, index);
            if (crf->filter) printf("\tfilter : '%s'\n", crf->filter);
            if (crf->textValue) printf("\ttextValue : '%s'\n", crf->textValue);
            if (printFlags & FI_EXTENDED)
            {
                for (size_t indexTV = 0; indexTV < crf->textValueArraySize; indexTV++)
                {
                    struct SearchedString* ss = &crf->textValueAsArray[indexTV];
                    printf("\t\tstr : '%s', words : ", ss->strOriginal.str);
                    for (size_t indexW = 0; indexW < ss->wordCount; indexW++)
                    {
                        printf("'%s' ", ss->wordDescriptors[indexW].str);
                    }
                    printf("\n");
                }
            }
            if (printFlags & FI_PROJECT_IDS)
            {
                printf("Project ids(%d):", (int)crf->groupIDListCount);
                for (size_t i = 0; i < crf->groupIDListCount; i++)
                {
                    printf("%d ", crf->groupIDList[i]);
                }
                printf("\n");
            }
        }
    }
    if (printFlags & FI_SEPARATION_BAR)
    {
        printf("******************************************************************\n");
    }
}