#ifndef COMPOSITESCORE_H
#define COMPOSITESCORE_H

#include <json_array.h>
#include <json_object.h>
#include <stdint.h>
#include <search_filter.h>

struct CompositeScore
{
    int32_t role;                                   // int `json:"r"`
    int32_t profile;                                // int `json:"p"`
    int16_t total;                                  // float32 `json:"t"`
    int16_t heuristicScore;                         // float32 `json:"hs"`
    int16_t companyScore;                           // float32 `json:"cs"`
    int16_t experienceScore;                        // float32 `json:"e"`
    int16_t skillsScore;                            // float32 `json:"s"`
    int16_t jobTitleScore;                          // float32 `json:"j"`
    int32_t relevantExperience;                     // float32 `json:"rel"`
    kvec_t(SearchFilterExplain) filterExplained;    // []SearchFilter `json:"filters"`
    const struct ProfileCached* srcProfile;         // values obtained from processing this profile. Not saved or freed
};

void initCompositeScore(struct CompositeScore* compositeScore);
void freeCompositeScore(struct CompositeScore* compositeScore);

struct json_object* marshallCompositeScore(const struct CompositeScore* compositeScore);
bool unmarshallCompositeScore(struct CompositeScore* compositeScore, const struct json_object* obj);

#define UNMARSHALL_COMPOSITESCORE(OBJ, VAR, RET) \
    UNMARSHALL_TYPE(unmarshallCompositeScore, OBJ, VAR, RET)

#define JSON_GET_COMPOSITESCORE_ARRAY(OBJ, KEY, ARRAY, RET) JSON_GET_ARRAY(struct CompositeScore, \
    UNMARSHALL_COMPOSITESCORE, initCompositeScore, freeCompositeScore, OBJ, KEY, ARRAY, RET)

#endif // COMPOSITESCORE_H
