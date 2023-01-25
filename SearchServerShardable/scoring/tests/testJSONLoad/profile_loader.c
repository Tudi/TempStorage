#include <profile_persistent.h>
#include <profile_cached.h>
#include <utils.h>
#include <k_utils.h>
#include <date_time.h>
#include <json_utils.h>
#include <json_specialized_array.h>
#include <json_tokener.h>
#include <linkhash.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <dirent.h> 
#include <logger.h>

unsigned int profilesLoadedCount = 0;
struct ProfileCached* cachedProfiles = NULL;
char** fileNames = NULL;

static void AllocMemoryNewProfile()
{
    if (cachedProfiles == NULL)
    {
        cachedProfiles = (struct ProfileCached*)malloc(sizeof(struct ProfileCached));
        fileNames = (char**)malloc(sizeof(char*));
    }
    else
    {
        cachedProfiles = (struct ProfileCached*)realloc(cachedProfiles, (profilesLoadedCount + 1) * sizeof(struct ProfileCached));
        fileNames = (char**)realloc(fileNames, sizeof(char*) * (profilesLoadedCount + 1));
    }
}

bool getFileContents_(const char *dir, const char* fileName, uint8_t** contents, size_t* size)
{
    if (fileName == NULL || contents == NULL || size == NULL)
    {
        return false;
    }
    *contents = NULL;
    *size = 0;
    char fullPath[16000];
    sprintf(fullPath, "%s/%s", dir, fileName);
    FILE* f = fopen(fullPath, "rt");
    if (f)
    {
        getFileContents(f, contents, size);
        fclose(f);
        return true;
    }
    else
    {
        printf("Could not open file : %s\n", fullPath);
    }
    return false;
}

void LoadProfileFromDirectory(const char* dir, const char *filename)
{
    uint8_t* fileContent;
    size_t contentByteCount;
    bool getres = getFileContents_(dir, filename, &fileContent, &contentByteCount);
    if (getres == false)
    {
        printf("Could not get file content : %s/%s\n", dir, filename);
        return;
    }
    struct ProfilePersistent persProfile;
    initProfilePersistent(&persProfile);

    struct json_tokener* jsonTokener = json_tokener_new();
    struct json_object* jsonObj = json_tokener_parse_ex(jsonTokener, (const char*)fileContent, (int)contentByteCount);
    if (jsonObj == NULL)
    {
        printf("JSON buffer (size = %zu) = %s\n\n", contentByteCount, fileContent);
    }
    else
    {
        if (unmarshallProfilePersistent(&persProfile, jsonObj))
        {
            AllocMemoryNewProfile();
            fileNames[profilesLoadedCount] = strdup(filename);
            struct ProfileCached* cachedProfile = &cachedProfiles[profilesLoadedCount];
            initProfileCached(cachedProfile);
            if (profilePersistentToProfileCached(cachedProfile, &persProfile))
            {
                profilesLoadedCount++;
            }
            else
            {
                printf("Error converting persistent profile to cached profile for %s\%s\n", dir, filename);
            }
        }
        else
        {
            printf("Could not convert %s/%s to profile\n", dir, filename);
        }
        json_object_put(jsonObj);
    }

    json_tokener_free(jsonTokener);

    freeProfilePersistent(&persProfile);
    free(fileContent);
}

void LoadProfilesFromDirectory(const char* dirName)
{
    struct dirent** namelist;
    int n;

    n = scandir(dirName, &namelist, NULL, alphasort);
    if (n < 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Failed to open directory %s for input files", dirName);
    }
    else 
    {
        while (n--) 
        {
            if (namelist[n]->d_name[0] != '.')
            {
                LoadProfileFromDirectory(dirName, namelist[n]->d_name);
            }
            free(namelist[n]);
        }
        free(namelist);
    }
}

void freeLoadedProfiles()
{
    for (int i = 0; i < profilesLoadedCount; i++)
    {
        freeProfileCached(&cachedProfiles[i]);
        free(fileNames[i]);
    }
    free(cachedProfiles);
    free(fileNames);
    profilesLoadedCount = 0;
    cachedProfiles = NULL;
    fileNames = NULL;
}

void LoadProfilesFromFiles()
{
    LoadProfilesFromDirectory("../JSON_Profile");
    if (profilesLoadedCount == 0)
    {
        LoadProfilesFromDirectory("./JSON_Profile");
    }
}
