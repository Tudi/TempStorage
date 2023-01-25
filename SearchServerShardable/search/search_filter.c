#include <search_filter.h>
#include <utils.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* name_Key      = "filter";
static const char* modifier_Key  = "modifier";
static const char* textValue_Key = "text_value";
static const char* codeValue_Key = "code_value";
static const char* rangeLow_Key  = "range_low";
static const char* rangeHigh_Key = "range_high";

void initSearchFilter(struct SearchFilter* searchFilter)
{
    searchFilter->name      = NULL;
    searchFilter->modifier  = NULL;
    searchFilter->textValue = NULL;
    searchFilter->codeValue = NULL;
    searchFilter->rangeLow  = 0;
    searchFilter->rangeHigh = 0;
}

void freeSearchFilter(struct SearchFilter* searchFilter)
{
    free(searchFilter->name);
    searchFilter->name = NULL;
    free(searchFilter->modifier);
    searchFilter->modifier = NULL;
    free(searchFilter->textValue);
    searchFilter->textValue = NULL;
    free(searchFilter->codeValue);
    searchFilter->codeValue = NULL;
}

struct json_object* marshallSearchFilter(const struct SearchFilter* searchFilter)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, name_Key, json_object_new_string(searchFilter->name));
        json_object_object_add(obj, modifier_Key, json_object_new_string(searchFilter->modifier));
        json_object_object_add(obj, textValue_Key, json_object_new_string(searchFilter->textValue));
        json_object_object_add(obj, codeValue_Key, json_object_new_string(searchFilter->codeValue));
        json_object_object_add(obj, rangeLow_Key, json_object_new_int(searchFilter->rangeLow));
        json_object_object_add(obj, rangeHigh_Key, json_object_new_int(searchFilter->rangeHigh));
    }

    return obj;
}

bool unmarshallSearchFilter(struct SearchFilter* searchFilter, const struct json_object* obj)
{
    freeSearchFilter(searchFilter);
    initSearchFilter(searchFilter);

    bool b1 = jsonGetString(obj, name_Key, &searchFilter->name);
    bool b2 = jsonGetString(obj, modifier_Key, &searchFilter->modifier);
    bool b3 = jsonGetString(obj, textValue_Key, &searchFilter->textValue);
    bool b4 = jsonGetString(obj, codeValue_Key, &searchFilter->codeValue);
    bool b5 = jsonGetInt32(obj, rangeLow_Key, &searchFilter->rangeLow);
    bool b6 = jsonGetInt32(obj, rangeHigh_Key, &searchFilter->rangeHigh);

    if(!(b1 && b2 && b3 && b4 && b5 && b6))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallSearchFilter() failed.");
        return false;
    }

    return true;
}

void initSearchFilterExplain(struct SearchFilterExplain* searchFilter)
{
    searchFilter->name = NULL;
    searchFilter->textValue = NULL;
}

void freeSearchFilterExplain(struct SearchFilterExplain* searchFilter)
{
    free(searchFilter->name);
    searchFilter->name = NULL;
    free(searchFilter->textValue);
    searchFilter->textValue = NULL;
}

struct json_object* marshallSearchFilterExplain(const struct SearchFilterExplain* searchFilter)
{
    struct json_object* obj = json_object_new_object();
    if (obj != NULL)
    {
        json_object_object_add(obj, name_Key, json_object_new_string(searchFilter->name));
        json_object_object_add(obj, textValue_Key, json_object_new_string(searchFilter->textValue));
    }

    return obj;
}

bool unmarshallSearchFilterExplain(struct SearchFilterExplain* searchFilter, const struct json_object* obj)
{
    freeSearchFilterExplain(searchFilter);
    initSearchFilterExplain(searchFilter);

    bool b1 = jsonGetString(obj, name_Key, &searchFilter->name);
    bool b2 = jsonGetString(obj, textValue_Key, &searchFilter->textValue);

    if (!(b1 && b2))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallSearchFilterExplain() failed.");
        return false;
    }

    return true;
}