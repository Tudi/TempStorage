#ifndef SEARCH_CRITERIA_H
#define SEARCH_CRITERIA_H

#include <search_filter.h>
#include <json_array.h>
#include <json_object.h>
#include <stdint.h>
#include <kvec.h>
#include <similarity_score.h>
#include <bitfield.h>

struct SearchCriteria
{
    int32_t role;                          // int `json:"role_code"`
    kvec_t(int32_t) localities;            // []int `json:"localities"`
    kvec_t(struct SearchFilter) filters;   // []SearchFilter `json:"filters"`
    int32_t organizationID;                // int `json:"organization_id"`
    SimilarityScores similarityScores;     // []SimilarityScores `json:similar_companies"`, `json:SimilarTitles"`, `json:SimilarTitles"`
    // Exists only for the sake of speed optimization
    BitField bfLocalities;                 // []int `json:"localities"`
};

void initSearchCriteria(struct SearchCriteria* searchCriteria);
void freeSearchCriteria(struct SearchCriteria* searchCriteria);

struct json_object* marshallSearchCriteria(const struct SearchCriteria* searchCriteria);
bool unmarshallSearchCriteria(struct SearchCriteria* searchCriteria, const struct json_object* obj,
    const uint8_t *networkPacket, int32_t networkBytes);

/// <summary>
/// Convert array of int into a bitfield. Consider this a speed optimization
/// </summary>
/// <param name="searchCriteria"></param>
void convertLocalitiesToBitfield(struct SearchCriteria* searchCriteria);
#endif // SEARCH_CRITERIA_H
