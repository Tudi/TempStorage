#ifndef DATE_TIME_H
#define DATE_TIME_H

#include <json_object.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

//
// This code uses the Posix date time which depends on the timezine set
// on the TZ environment variable.
// TZ must be always set to "UTC".
//

time_t getDate1_time_t();

struct json_object* marshallTime(time_t t);
bool unmarshallTime(time_t* t, const struct json_object* obj);

uint8_t* timeToBinary(uint8_t* byteStream, time_t t);
const uint8_t* binaryToTime(const uint8_t* byteStream, time_t* t);

#define SKIP_BINARY_TIME(BYTESTREAM) ((BYTESTREAM) + sizeof(time_t))
#define TIME_BINARY_SIZE ((uint32_t) sizeof(time_t))

#endif // DATE_TIME_H
