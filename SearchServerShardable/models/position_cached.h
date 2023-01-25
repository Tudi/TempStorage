#ifndef POSITION_CACHED_H
#define POSITION_CACHED_H

#include <position_persistent.h>

#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <date_time.h>

struct PositionCached
{
    char* companyName;
    char* title;
    time_t startDate;
    time_t endDate;
    int32_t companyId;
    int32_t parentTitletId;
};

void initPositionCached(struct PositionCached* position);
void freePositionCached(struct PositionCached* position);

const uint8_t* binaryToPositionCached(const uint8_t* byteStream, struct PositionCached* position);
bool positionPersistentToPositionCached(struct PositionCached* positionCached,
    const struct PositionPersistent* positionPersistent);

/// <summary>
/// function converted from "position.go". Constant values and checks came from there. 
/// </summary>
/// <param name="positionCached"></param>
/// <returns>days since employed</returns>
#define NUMBER_SECONDS_IN_DAY	                      (24 * 60 * 60)
#define NUMBER_DAYS_IN_YEAR                           (365)
#define CONVERT_SECONDS_XP_TO_MONTHS(type, seconds)   (((type)(seconds) * (type)12) / (type)NUMBER_SECONDS_IN_DAY / NUMBER_DAYS_IN_YEAR)
#define CONVERT_DAYS_XP_TO_MONTHS(type, days)         (((type)(days) * (type)12) / (type)NUMBER_DAYS_IN_YEAR)
time_t GetPositionCachedDuration(const struct PositionCached* positionCached, const time_t timeStamp);

/// <summary>
/// Check if a position meets the requirements to be considered 'current'
/// </summary>
/// <param name="pos"></param>
/// <returns>1 if current</returns>
int isPositionCurrent(const struct PositionCached* pos);

#define BINARY_TO_POSITIONCACHED_ARRAY(ARRAY, BYTESTREAM, OFFSET) \
    BINARY_TO_ARRAY(struct PositionCached, ARRAY, binaryToPositionCached, \
        initPositionCached, freePositionCached, BYTESTREAM, OFFSET)

#endif // POSITION_CACHED_H
