#include <date_time.h>
#include <logger.h>
#include <string.h>

//
// Variables
//

static struct tm date1_tm = { .tm_sec = 0, .tm_min = 0, .tm_hour = 0,
    .tm_mday = 1, .tm_mon = 0, .tm_year = -1899, .tm_wday = 0, .tm_yday = 0, .tm_isdst = 0 };

//
// External interface
//

time_t getDate1_time_t()
{
    static time_t date1_time_t = 0;

    if(date1_time_t == 0) {
        date1_time_t = mktime(&date1_tm);
    }

    return date1_time_t;
}

// "YYYY-MM-DDTHH:MM:SSZ"
static const size_t bufSize = 21;

const char* format = "%04Y-%m-%dT%TZ";

struct json_object* marshallTime(time_t t)
{
    char buf[bufSize];
    struct tm tms;

    localtime_r(&t, &tms);
    strftime(buf, bufSize, format, &tms);

    return json_object_new_string(buf);
}

bool unmarshallTime(time_t* t, const struct json_object* obj)
{
    struct json_object* auxObj = (struct json_object*) obj;

	const char *str = json_object_get_string(auxObj);
    if(str == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallTime_t() - non-existent string.");
        return false;
    }

    struct tm tms;

    if(strptime(str, format, &tms) == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallTime_t() - invalid date string.");
        return false;
    }

    tms.tm_isdst = 0;
    *t = mktime(&tms);

    return true;
}

uint8_t* timeToBinary(uint8_t* byteStream, time_t t)
{
    memcpy(byteStream, &t, sizeof(t));
    byteStream += sizeof(t);

    return byteStream;
}

const uint8_t* binaryToTime(const uint8_t* byteStream, time_t* t)
{
    memcpy(t, byteStream, sizeof(*t));
    byteStream += sizeof(*t);

    return byteStream;
}
