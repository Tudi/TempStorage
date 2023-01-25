#ifndef SEARCH_FILTER_H
#define SEARCH_FILTER_H

#include <json_array.h>
#include <json_object.h>
#include <stdint.h>

struct SearchFilter
{
	char* name;        // string `json:"filter"`
	char* modifier;    // string `json:"modifier"`
	char* textValue;   // string `json:"text_value"`
	char* codeValue;   // string `json:"code_value"`
	int32_t rangeLow;  // int `json:"range_low"`
	int32_t rangeHigh; // int `json:"range_high"`
};

void initSearchFilter(struct SearchFilter* searchFilter);
void freeSearchFilter(struct SearchFilter* searchFilter);

struct json_object* marshallSearchFilter(const struct SearchFilter* searchFilter);
bool unmarshallSearchFilter(struct SearchFilter* searchFilter, const struct json_object* obj);

typedef struct SearchFilterExplain
{
	char* name;
	char* textValue;
}SearchFilterExplain;

void initSearchFilterExplain(struct SearchFilterExplain* searchFilter);
void freeSearchFilterExplain(struct SearchFilterExplain* searchFilter);
struct json_object* marshallSearchFilterExplain(const struct SearchFilterExplain* searchFilter);
bool unmarshallSearchFilterExplain(struct SearchFilterExplain* searchFilter, const struct json_object* obj);

#define UNMARSHALL_SEARCHFILTER(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallSearchFilter, OBJ, VAR, RET)

#define JSON_GET_SEARCHFILTER_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct SearchFilter, \
    UNMARSHALL_SEARCHFILTER, initSearchFilter, freeSearchFilter, OBJ, KEY, ARRAY, RET)

#define UNMARSHALL_SEARCHFILTEREXPLAIN(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallSearchFilterExplain, OBJ, VAR, RET)

#endif // SEARCH_FILTER_H
