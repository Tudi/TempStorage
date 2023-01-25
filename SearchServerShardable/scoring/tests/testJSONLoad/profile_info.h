#ifndef _PROFILE_INFO_H_
#define _PROFILE_INFO_H_

enum ProfileInfoFlags
{
    PI_BASIC = 1 << 0,        // print profile data without going onto details
    PI_POSITIONS = 1 << 1,    // print profile positions
    PI_EXTENDED = 1 << 2,     // rarely used values
    PI_SEPARATION_BAR = 1 << 3,
    PI_MESSAGED = 1 << 4,
    PI_GROUPS = 1 << 5,
    PI_PROJECTS = 1 << 6,
    PI_ACTIONED = 1 << 7,
    PI_SKILLS = 1 << 8,
    PI_ALL_FLAGS = PI_BASIC | PI_POSITIONS| PI_EXTENDED| PI_MESSAGED | PI_GROUPS | PI_PROJECTS | PI_ACTIONED | PI_SKILLS
};

/// <summary>
/// Print profile related information in a humanly readable manner
/// </summary>
/// <param name="prof"></param>
/// <param name="printFlags"></param>
void PrintProfile(const struct ProfileCached* prof, int printFlags);

#endif
