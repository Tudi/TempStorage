#include <system_info_data.h>
#include <json_utils.h>
#include <logger.h>

static const char* config_Key          = "config";
static const char* numConnections_Key  = "client_connections";
static const char* persistentItems_Key = "persistent_items";
static const char* profiles_Key        = "profiles";
static const char* companies_Key       = "companies";
static const char* minId_Key           = "min";
static const char* maxId_Key           = "max";
static const char* totalIds_Key        = "total";

void initSystemInfoData(struct SystemInfoData* info)
{
    info->config.numConnections = 0;

    info->persistentItems.profiles.minId = INT32_MAX;
    info->persistentItems.profiles.maxId = 0;
    info->persistentItems.profiles.total = 0;

    info->persistentItems.companies.minId = INT32_MAX;
    info->persistentItems.companies.maxId = 0;
    info->persistentItems.companies.total = 0;
}

void freeSystemInfoData(struct SystemInfoData* profileTag) { }

struct json_object* marshallSystemInfoData(const struct SystemInfoData* info)
{
    // Root

    struct json_object* infoObj = NULL;

    infoObj = json_object_new_object();
    if(infoObj == NULL) { goto marshallSystemInfoData_cleanup; }

    // config section

    struct json_object* configObj = json_object_new_object();
    if(configObj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(infoObj, config_Key, configObj);

    struct json_object* obj = json_object_new_int(info->config.numConnections);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(configObj, numConnections_Key, obj);

    // persistentItems section

    struct json_object* persistentItemsObj = json_object_new_object();
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(infoObj, persistentItems_Key, persistentItemsObj);

    // profiles section

    struct json_object* profilesObj = json_object_new_object();
    if(profilesObj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(persistentItemsObj, profiles_Key, profilesObj);

    obj = json_object_new_int(info->persistentItems.profiles.minId);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(profilesObj, minId_Key, obj);

    obj = json_object_new_int(info->persistentItems.profiles.maxId);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(profilesObj, maxId_Key, obj);

    obj = json_object_new_int64(info->persistentItems.profiles.total);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(profilesObj, totalIds_Key, obj);

    // companies section

    struct json_object* companiesObj = json_object_new_object();
    if(companiesObj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(persistentItemsObj, companies_Key, companiesObj);

    obj = json_object_new_int(info->persistentItems.companies.minId);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(companiesObj, minId_Key, obj);

    obj = json_object_new_int(info->persistentItems.companies.maxId);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(companiesObj, maxId_Key, obj);

    obj = json_object_new_int64(info->persistentItems.companies.total);
    if(obj == NULL) { goto marshallSystemInfoData_cleanup; }

    json_object_object_add(companiesObj, totalIds_Key, obj);

    return infoObj;
    
marshallSystemInfoData_cleanup:
    json_object_put(infoObj);
    return NULL;
}

bool unmarshallSystemInfoData(struct SystemInfoData* info, const struct json_object* obj)
{
    freeSystemInfoData(info);
    initSystemInfoData(info);

    // Root

    struct json_object* configObj          = NULL;
    struct json_object* persistentItemsObj = NULL;

    bool b1 = jsonGetObject(obj, config_Key, &configObj);
    bool b2 = jsonGetObject(obj, persistentItems_Key, &persistentItemsObj);

    if(!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    // persistentItems section

    struct json_object* profilesObj  = NULL;
    struct json_object* companiesObj = NULL;

    b1 = jsonGetObject(persistentItemsObj, profiles_Key, &profilesObj);
    b2 = jsonGetObject(persistentItemsObj, companies_Key, &companiesObj);

    if(!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    // Data

    b1 = jsonGetUint16(configObj, numConnections_Key, &info->config.numConnections);

    b2 = jsonGetInt32(profilesObj, minId_Key, &info->persistentItems.profiles.minId);
    bool b3 = jsonGetInt32(profilesObj, maxId_Key, &info->persistentItems.profiles.maxId);
    bool b4 = jsonGetUint32(profilesObj, totalIds_Key, &info->persistentItems.profiles.total);

    bool b5 = jsonGetInt32(companiesObj, minId_Key, &info->persistentItems.companies.minId);
    bool b6 = jsonGetInt32(companiesObj, maxId_Key, &info->persistentItems.companies.maxId);
    bool b7 = jsonGetUint32(companiesObj, totalIds_Key, &info->persistentItems.companies.total);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Parsing failed.");
        return false;
    }

    return true;
}
