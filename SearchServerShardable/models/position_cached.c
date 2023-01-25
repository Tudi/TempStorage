#include <position_cached.h>
#include <date_time.h>
#include <utils.h>
#include <stdio.h>
#include <stdlib.h>

void initPositionCached(struct PositionCached* position)
{
    position->companyName    = NULL;
    position->title          = NULL;
    position->startDate      = getDate1_time_t();
    position->endDate        = getDate1_time_t();
    position->companyId      = 0;
    position->parentTitletId = 0;
}

void freePositionCached(struct PositionCached* position)
{
    free(position->companyName);
    position->companyName = NULL;
    free(position->title);
    position->title = NULL;
}

const uint8_t* binaryToPositionCached(const uint8_t* byteStream, struct PositionCached* position)
{
    freePositionCached(position);
    initPositionCached(position);

    const uint8_t* offset = byteStream;

    offset = binaryToLowerString(offset, &position->companyName);
    if(offset == NULL) { return NULL; }

    offset = binaryToLowerString(offset, &position->title);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &position->startDate);
    if(offset == NULL) { return NULL; }

    offset = binaryToTime(offset, &position->endDate);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &position->companyId);
    if(offset == NULL) { return NULL; }

    offset = binaryToInt32(offset, &position->parentTitletId);
    if (offset == NULL) { return NULL; }

    // Skipping rest of binary PositionPersistent.

    offset = skipBinaryString(offset); // uuid
    if(offset == NULL) { return NULL; }

    offset = skipBinaryString(offset); // titleCorrected
    if(offset == NULL) { return NULL; }

    offset = skipBinaryString(offset); // source
    if(offset == NULL) { return NULL; }

    offset = skipBinaryString(offset); // locality
    if(offset == NULL) { return NULL; }

    offset = SKIP_BINARY_INT32(offset); // parentCompanyId

    offset = skipBinaryString(offset); // description
    if(offset == NULL) { return NULL; }

    offset = skipBinaryString(offset); // seniority
    if(offset == NULL) { return NULL; }

    offset = SKIP_BINARY_BOOLEAN(offset); // modifiedInLoad

    offset = SKIP_BINARY_INT32(offset); // parentTitleId

    return offset;
}

bool positionPersistentToPositionCached(struct PositionCached* positionCached,
    const struct PositionPersistent* positionPersistent)
{
    positionCached->companyName = strdupLower(positionPersistent->companyName);
    if(positionCached->companyName == NULL) { return false; }

    positionCached->title = strdupLower(positionPersistent->titleCorrected);
    if(positionCached->title == NULL) { return false; }

    positionCached->startDate     = positionPersistent->startDate;
    positionCached->endDate       = positionPersistent->endDate;
    positionCached->companyId     = positionPersistent->companyId;
    positionCached->parentTitletId = positionPersistent->parentTitleId;

    return true;
}

/// <summary>
/// function converted from "position.go". Constant values and checks came from there. 
/// Need to make sure that data sent/loaded also uses the same "IsZero" checks
/// </summary>
/// <param name="positionCached"></param>
/// <returns>days since employed</returns>
time_t GetPositionCachedDuration(const struct PositionCached* positionCached,
    const time_t timeStamp)
{
    time_t startStamp = positionCached->startDate;
    if (startStamp == getDate1_time_t()) { return 0; }

    time_t endStamp = positionCached->endDate;
    if (endStamp == getDate1_time_t()) { endStamp = timeStamp; }

    time_t durationSec = endStamp - startStamp;
    time_t durationDay = durationSec / NUMBER_SECONDS_IN_DAY;

    // maybe for V2 move part of this to positionCached generation
    struct tm startDate, endDate;

    localtime_r(&positionCached->startDate, &startDate);
    localtime_r(&positionCached->endDate, &endDate);
    int isDefaultStartValue = (startDate.tm_mday == 1) && (startDate.tm_mon == 0);
    int isDefaultEndValue = (endDate.tm_mday == 31) && (endDate.tm_mon == 11);
    if (isDefaultStartValue && isDefaultEndValue && startDate.tm_year != endDate.tm_year)
    {
        return durationDay - NUMBER_DAYS_IN_YEAR;
    }

    return durationDay;
}

int isPositionCurrent(const struct PositionCached* pos)
{
    return (pos->endDate == getDate1_time_t()) && (pos->startDate != getDate1_time_t());
}
