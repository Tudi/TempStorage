#ifndef _SIMILARITY_SCORE_H_
#define _SIMILARITY_SCORE_H_

#include <stdbool.h>
#include <search.h>
#include <json_object.h>

typedef enum NetworkDataBlockTypes
{
    NDBT_UNUSED_VALUE = 0,
    NDBT_COMPANY_SIMILARITY_SCORE = 1, 
    NDBT_INDUSTRY_SIMILARITY_SCORE = 2,
    NDBT_TITLE_SIMILARITY_SCORE = 3,
    NDBT_MAX_VALUE,
}NetworkDataBlockTypes;

#pragma pack(push, 1) // network packets need to be byte alligned always
typedef struct NetworkSimilarityScoreData
{
    uint32_t id; // company / industry / title ID
    uint16_t score;
}NetworkSimilarityScoreData;

typedef struct NetworkSimilarityScoreHeader
{
    uint32_t size; // given in bytes
    uint32_t type; // data block type
    uint32_t blockCount; // list size
    NetworkSimilarityScoreData data[0]; // list of values
}NetworkSimilarityScoreHeader;
#pragma pack(pop)

#define MAX_SCORE_VALUE_ACCEPTED 10000
#define MAX_ID_VALUE_ACCEPTED_FROM_NETWORK 110000000

typedef struct SimilarityScoreFlatMap
{
    uint16_t* scores; // right now for 7M companies, Will use direct lookup map
    int          maxId;
    uint32_t scoresCount;
}SimilarityScoreFlatMap;

typedef struct SimilarityScores
{
    SimilarityScoreFlatMap companyScores; // right now for 7M companies, I will use direct lookup map
    SimilarityScoreFlatMap industryScores;
    SimilarityScoreFlatMap titleScores;
}SimilarityScores;

void initSimilarityScore(SimilarityScores* similarityScore);
void freeSimilarityScore(SimilarityScores* similarityScore);

/// <summary>
/// Extract similarity scores from a binary packet
/// </summary>
/// <param name="similarityScore"></param>
/// <returns>true if all required fields were found in the search string</returns>
bool unmarshallSimilarityScoresBinary(SimilarityScores* similarityScore, const uint8_t *networkPacket, int32_t bytesRemain);

/// <summary>
/// serialize binary structure into JSON format
/// </summary>
/// <param name="similarityScore"></param>
/// <returns>false if it failed to construct the JSON</returns>
bool marshallSimilarityScores(const SimilarityScores* similarityScore, struct json_object *addTo);

/// <summary>
/// API to obtain the similarity score of a specific company
/// </summary>
/// <param name="companyId">The company ID for which we are looking for the most similar company</param>
/// <param name="similarityScore">The score for the most similar company</param>
/// <returns>structure of the company that best matches the searched company</returns>
const float GetCompanySimilarityScore(const int32_t companyId, const SimilarityScores* similarityScore);
const float GetIndustrySimilarityScore(const int32_t industryId, const SimilarityScores* similarityScore);
const float GetTitleSimilarityScore(int32_t profileId, const int32_t titleId, const SimilarityScores* similarityScore);

/// <summary>
/// Used for testing
/// </summary>
/// <param name="similarityScore"></param>
/// <param name=""></param>
void addScoreToMap(SimilarityScoreFlatMap* scoreMap, const int32_t companyId, const uint16_t score);
#endif
