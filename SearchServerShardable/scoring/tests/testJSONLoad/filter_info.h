#ifndef _FILTER_INFO_H_
#define _FILTER_INFO_H_

enum FilterInfoFlags
{
    FI_BASIC = 1,           // print profile data without going onto details
    FI_FILTER_TEXTS = 2,    // print profile positions
    FI_EXTENDED = 4,        // rarely used values
    FI_SEPARATION_BAR = 8,
    FI_PROJECT_IDS = 16,
};

/// <summary>
/// Print filter related information in a humanly readable manner
/// </summary>
/// <param name="prof"></param>
/// <param name="printFlags"></param>
void PrintFilter(struct PerformFilter* filter, int printFlags);
#endif