#include <composite_score.h>
#include <utils.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* role_Key               = "r";
static const char* profile_Key            = "p";
static const char* total_Key              = "t";
static const char* heuristicScore_Key     = "hs";
static const char* companyScore_Key       = "cs";
static const char* experienceScore_Key    = "e";
static const char* skillsScore_Key        = "s";
static const char* jobTitleScore_Key      = "j";
static const char* relevantExperience_Key = "rel";
static const char* filtersExplained_Key   = "filters";

void initCompositeScore(struct CompositeScore* compositeScore)
{
    compositeScore->role = 0;
    compositeScore->profile = 0;
    compositeScore->total = 0;
    compositeScore->heuristicScore = 0;
    compositeScore->companyScore = 0;
    compositeScore->experienceScore = 0;
    compositeScore->skillsScore = 0;
    compositeScore->jobTitleScore = 0;
    compositeScore->relevantExperience = 0;
    kv_init(compositeScore->filterExplained);
    compositeScore->srcProfile = NULL;
}

void freeCompositeScore(struct CompositeScore* compositeScore) 
{ 
    for (size_t index = 0; index < kv_size(compositeScore->filterExplained); index++)
    {
        freeSearchFilterExplain(&kv_A(compositeScore->filterExplained, index));
    }
    kv_destroy(compositeScore->filterExplained);
    compositeScore->srcProfile = NULL;
}

struct json_object* marshallCompositeScore(const struct CompositeScore* compositeScore)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, role_Key, json_object_new_int(compositeScore->role));
        json_object_object_add(obj, profile_Key, json_object_new_int(compositeScore->profile));
        json_object_object_add(obj, total_Key, json_object_new_int(compositeScore->total));
        json_object_object_add(obj, heuristicScore_Key,
            json_object_new_int(compositeScore->heuristicScore));
        json_object_object_add(obj, companyScore_Key,
            json_object_new_int(compositeScore->companyScore));
        json_object_object_add(obj, experienceScore_Key,
            json_object_new_int(compositeScore->experienceScore));
        json_object_object_add(obj, skillsScore_Key,
            json_object_new_int(compositeScore->skillsScore));
        json_object_object_add(obj, jobTitleScore_Key,
            json_object_new_int(compositeScore->jobTitleScore));
        json_object_object_add(obj, relevantExperience_Key,
            json_object_new_int(compositeScore->relevantExperience));

        struct json_object* filterExplainedArray = json_object_new_array();
        for (size_t index = 0; index < kv_size(compositeScore->filterExplained); index++)
        {
            json_object_array_add(filterExplainedArray, marshallSearchFilterExplain(&kv_A(compositeScore->filterExplained, index)));
        }
        json_object_object_add(obj, filtersExplained_Key, filterExplainedArray);
    }

    return obj;
}

bool unmarshallCompositeScore(struct CompositeScore* compositeScore, const struct json_object* obj)
{
    freeCompositeScore(compositeScore);
    initCompositeScore(compositeScore);

    bool b1 = jsonGetInt32(obj, role_Key, &compositeScore->role);
    bool b2 = jsonGetInt32(obj, profile_Key, &compositeScore->profile);
    bool b3 = jsonGetInt16(obj, total_Key, &compositeScore->total);
    bool b4 = jsonGetInt16(obj, heuristicScore_Key, &compositeScore->heuristicScore);
    bool b5 = jsonGetInt16(obj, companyScore_Key, &compositeScore->companyScore);
    bool b6 = jsonGetInt16(obj, experienceScore_Key, &compositeScore->experienceScore);
    bool b7 = jsonGetInt16(obj, skillsScore_Key, &compositeScore->skillsScore);
    bool b8 = jsonGetInt16(obj, jobTitleScore_Key, &compositeScore->jobTitleScore);
    bool b9 = jsonGetInt32(obj, relevantExperience_Key, &compositeScore->relevantExperience);

    bool b10;
    JSON_GET_ARRAY(struct SearchFilterExplain, UNMARSHALL_SEARCHFILTEREXPLAIN, \
        initSearchFilterExplain, freeSearchFilterExplain, obj, filtersExplained_Key, compositeScore->filterExplained, b10);

    if(!(b1 && b2 && b3 && b4 && b5 && b6 && b7 && b8 && b9 && b10))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallCompositeScore() failed.");
        return false;
    }

    return true;
}
