#ifndef _PROFILE_LOADER_H_
#define _PROFILE_LOADER_H_

void LoadProfilesFromFiles();
void freeLoadedProfiles();

extern unsigned int profilesLoadedCount;
extern struct ProfileCached* cachedProfiles;
extern char** fileNames;

#endif