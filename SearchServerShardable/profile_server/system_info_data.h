#ifndef SYSTEM_INFO_DATA_H
#define SYSTEM_INFO_DATA_H

#include <json_object.h>
#include <stdbool.h>

struct SystemInfoData
{
    struct {
        uint16_t numConnections; // int   `json:client_connections` 
    } config;                    //       `json:config` 

    struct {
        struct {
            int32_t minId;       // int   `json:min` 
            int32_t maxId;       // int   `json:max` 
            uint32_t total;      // int64 `json:total` 
        } profiles;              //       `json:profiles` 

        struct {
            int32_t minId;       // int   `json:min` 
            int32_t maxId;       // int   `json:max` 
            uint32_t total;      // int64 `json:total` 
        } companies;             //       `json:companies` 
    } persistentItems;           //       `json:persistentItems` 
};

void initSystemInfoData(struct SystemInfoData* info);
void freeSystemInfoData(struct SystemInfoData* info);

struct json_object* marshallSystemInfoData(const struct SystemInfoData* info);
bool unmarshallSystemInfoData(struct SystemInfoData* info, const struct json_object* obj);

#endif // SYSTEM_INFO_DATA_H
