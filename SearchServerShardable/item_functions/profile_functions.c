#include <profile_functions.h>
#include <profile_cached.h>
#include <profile_persistent.h>
#include <profile_definitions.h>
#include <logger.h>
#include <json_array.h>
#include <k_utils.h>
#include <string.h>
#include <ctype.h>

//
// Prototypes
//

// ProfilePersistent

static DaosId_t getProfilePersistentId(const void* item);
static void freeProfilePersistentItem(void* item);
static uint8_t* profilePersistentItemToBinary(uint8_t* byteStream, const void* item);
static int binaryToProfilePersistentItem(const uint8_t* byteStream, void** item, int fileVersion);
static uint32_t profilePersistentItemBinarySize(const void* item);
static struct json_object* profilePersistentToJson(const void* item);
static int jsonToProfilePersistent(void** item, const struct json_object* obj);
static int jsonToProfilePersistentList(void** lst, const struct json_object* obj, const char* jsonKey);
static void freeProfilePersistentList(void* lst);
static uint32_t profilePersistentListCount(const void* lst);
static void* getItemFromProfilePersistentList(void* lst, uint32_t index);
static bool profilePersistentShouldBeCached(const void* item);
static bool isProfilePersistentFile(const char* filename);
static bool isProfileDestinationServerCorrect(const void* item, DaosCount_t numServers, const DaosId_t serverId);

// ProfileCached

static DaosId_t getProfileCachedId(const void* item);
static void* generateProfileCached(const void* item);
static void freeProfileCachedList(void** items, uint32_t numItems);
static void freeProfileCachedItem(void* item);
static int binaryToProfileCachedItem(const uint8_t* byteStream, void** item, int fileVersion);
static DaosCount_t generateProfileCacheIndex(DaosId_t id, DaosCount_t numServers);


//
// Variables
//

static ItemFunctions_t profileFunctions = 
{
    // Persistent item

    .getPersistentItemId          = getProfilePersistentId,
    .freePersistentItem           = freeProfilePersistentItem,
    .persistentItemToBinary       = profilePersistentItemToBinary,
    .binaryToPersistentItem       = binaryToProfilePersistentItem,
    .persistentItemBinarySize     = profilePersistentItemBinarySize,
    .persistentItemToJson         = profilePersistentToJson,
    .jsonToPersistentItem         = jsonToProfilePersistent,
    .jsonToPersistentList         = jsonToProfilePersistentList,
    .freePersistentList           = freeProfilePersistentList,
    .persistentListCount          = profilePersistentListCount,
    .getItemFromPersistentList    = getItemFromProfilePersistentList,
    .persistentItemShouldBeCached = profilePersistentShouldBeCached,
    .isItemFile                   = isProfilePersistentFile,
    .isDestinationServerCorrect   = isProfileDestinationServerCorrect,

    // Cached item

    .getCachedItemId    = getProfileCachedId,
    .generateCachedItem = generateProfileCached,
    .freeCachedList     = freeProfileCachedList,
    .freeCachedItem     = freeProfileCachedItem,
    .binaryToCachedItem = binaryToProfileCachedItem,
    .generateCacheIndex = generateProfileCacheIndex
};

//
// External interface
//

const ItemFunctions_t* getProfileFunctions()
{
    return &profileFunctions;
}

//
// Local functions
//

//
// Persistent item
//

static DaosId_t getProfilePersistentId(const void* item)
{
    const struct ProfilePersistent* profile = (const struct ProfilePersistent*) item;

    return profile->id;
}

static void freeProfilePersistentItem(void* item)
{
    struct ProfilePersistent* profile = (struct ProfilePersistent*) item;
    if(profile != NULL)
    {
        freeProfilePersistent(profile);
        free(profile);
    }
}

static uint8_t* profilePersistentItemToBinary(uint8_t* byteStream, const void* item)
{
    const struct ProfilePersistent* profile = (const struct ProfilePersistent*) item;

    return profilePersistentToBinary(byteStream, profile);
}

static int binaryToProfilePersistentItem(const uint8_t* byteStream, void** item, int fileVersion)
{
    struct ProfilePersistent* profile = (struct ProfilePersistent*) malloc(sizeof(struct ProfilePersistent));
    if(profile == NULL) { return 1; }

    initProfilePersistent(profile);

    const uint8_t* offset = binaryToProfilePersistent(byteStream, profile, fileVersion);
    if(offset == NULL)
    {
        free(profile);
        return 2;
    }

    *item = profile;

    return 0;
}

static uint32_t profilePersistentItemBinarySize(const void* item)
{
    const struct ProfilePersistent* profile = (const struct ProfilePersistent*) item;

    return profilePersistentBinarySize(profile);
}

static struct json_object* profilePersistentToJson(const void* item)
{
    const struct ProfilePersistent* profile = (const struct ProfilePersistent*) item;

    return marshallProfilePersistent(profile);
}

static int jsonToProfilePersistent(void** item, const struct json_object* obj)
{
    struct ProfilePersistent* profile = (struct ProfilePersistent*) malloc(sizeof(struct ProfilePersistent));
    if(profile == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.", sizeof(struct ProfilePersistent));
        return 1;
    }

    initProfilePersistent(profile);

    bool ret = unmarshallProfilePersistent(profile, obj);
    if(ret == false)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallProfilePersistent() failed.");
        freeProfilePersistentItem(profile);
        return 2;
    }

    *item = profile;

    return 0;
}

static int jsonToProfilePersistentList(void** lst, const struct json_object* obj, const char* jsonKey)
{
    bool ret = false;
    ProfilePersistentKvec_t* profiles = (ProfilePersistentKvec_t*) malloc(sizeof(ProfilePersistentKvec_t));
    if(profiles == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: malloc(size = %zu bytes) failed.", sizeof(ProfilePersistentKvec_t));
        return 1;
    }

    kv_init(*profiles);
    JSON_GET_PROFILEPERSISTENT_ARRAY(obj, jsonKey, *profiles, ret);

    if(ret == false)
    {
        freeProfilePersistentList(profiles);
        LOG_MESSAGE(DEBUG_LOG_MSG, "Error: JSON_GET_PROFILEPERSISTENT_ARRAY() failed.");
        return 2;
    }

    *lst = profiles;

    return 0;
}

static void freeProfilePersistentList(void* lst)
{
    ProfilePersistentKvec_t* profiles = lst;

    FREE_KVEC_ADDR(*profiles, freeProfilePersistent);
    free(profiles);
}

static uint32_t profilePersistentListCount(const void* lst)
{
    const ProfilePersistentKvec_t* profiles = lst;

    return kv_size(*profiles);
}

static void* getItemFromProfilePersistentList(void* lst, uint32_t index)
{
    ProfilePersistentKvec_t* profiles = lst;

    return &(kv_A(*profiles, index));
}

static bool profilePersistentShouldBeCached(const void* item)
{
    const struct ProfilePersistent* profilePersistent = (const struct ProfilePersistent*) item;

    return profilePersistent->logicalDelete == false;
}

static bool isProfilePersistentFile(const char* filename)
{
    static const size_t profileFilePrefixLength = strlen(PROFILE_FILE_PREFIX);
    static const size_t profileFileExtensionLength = strlen(PROFILE_FILE_EXTENSION);
    static const size_t profileFilenameLength = profileFilePrefixLength + PROFILE_NUM_ID_DIGITS
        + 1 + profileFileExtensionLength;

    if(strlen(filename) != profileFilenameLength) { return false; }

    const char* filenamePtr = filename;

    if(strncmp(filenamePtr, PROFILE_FILE_PREFIX, profileFilePrefixLength)) { return false; }

    filenamePtr += profileFilePrefixLength;

    uint16_t i = 0;
    for(; i < PROFILE_NUM_ID_DIGITS; ++i) {
        if(!isdigit(filenamePtr[i])) { return false; }
    }

    filenamePtr += PROFILE_NUM_ID_DIGITS;

    return !strcmp(filenamePtr, "." PROFILE_FILE_EXTENSION);
}

//
// Cached item
//

static DaosId_t getProfileCachedId(const void* item)
{
    const struct ProfileCached* profileCached = (const struct ProfileCached*) item;

    return profileCached->id;
}

static void* generateProfileCached(const void* item)
{
    const struct ProfilePersistent* profilePersistent = (const struct ProfilePersistent*) item;

    struct ProfileCached* profileCached
        = (struct ProfileCached*) malloc(sizeof(struct ProfileCached));
    if(profileCached == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (profilePersistent.id = %lu) - malloc(size"
            " = %zu bytes) failed.", profilePersistent->id, sizeof(struct ProfileCached));
        return NULL;
    }

    initProfileCached(profileCached);

    if(!profilePersistentToProfileCached(profileCached, profilePersistent))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: (profilePersistent.id = %lu)"
            " - profilePersistentToProfileCached() failed.", profilePersistent->id);
        freeProfileCached(profileCached);
        free(profileCached);
        return NULL;
    }

    return profileCached;
}

static void freeProfileCachedList(void** items, uint32_t numItems)
{
    struct ProfileCached** list = (struct ProfileCached**) items;

    if(list == NULL) { return; }

    for(uint32_t i = 0; i < numItems; ++i)
    {
        if(list[i] != NULL)
        {
            freeProfileCached(list[i]);
            free(list[i]);
        }
    }

    free(list);
}

static void freeProfileCachedItem(void* item)
{
    struct ProfileCached* profile = (struct ProfileCached*) item;
    if(profile != NULL)
    {
        freeProfileCached(profile);
        free(profile);
    }
}

static int binaryToProfileCachedItem(const uint8_t* byteStream, void** item, int fileVersion)
{
    bool logicalDelete = false;

    const uint8_t* offset = binaryToBoolean(byteStream, &logicalDelete);
    if(offset == NULL) { return 1; }

    if(logicalDelete == true) { return 0; }

    struct ProfileCached* profile = (struct ProfileCached*) malloc(sizeof(struct ProfileCached));
    if(profile == NULL) { return 2; }

    initProfileCached(profile);

    offset = binaryToProfileCached(byteStream, profile);
    if (offset == NULL)
    {
        free(profile);
        return 3;
    }

    *item = profile;

    return 0;
}

static DaosCount_t generateProfileCacheIndex(DaosId_t id, DaosCount_t numServers)
{
    return id / numServers;
}

static bool isProfileDestinationServerCorrect(const void* item, DaosCount_t numServers, const DaosId_t serverId)
{
    const struct ProfilePersistent* profilePersistent = (const struct ProfilePersistent*)item;
    int expectedServerId = (profilePersistent->id % numServers);

    return expectedServerId == serverId;
}
