#include <similarity_score.h>
#include <logger.h>
#include <json_utils.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>

static const char *similarCompaniesScores_Key = "similar_companies";
// static const char* similarTitleScores_Key = "similar_titles";
static const char *similarIndustryScores_Key = "similar_industries";
static const char *id_Key = "id";
static const char *value_Key = "value";

//#define _PRINT_WHAT_IS_HAPPENING_IN_SIMILARITY_PARSING_
#ifdef _PRINT_WHAT_IS_HAPPENING_IN_SIMILARITY_PARSING_
#define DSSP_LOG_MESSAGE(...) LOG_MESSAGE(__VA_ARGS__)
#else
#define DSSP_LOG_MESSAGE(...)
#endif

void initSimilarityScore(struct SimilarityScores *similarityScore)
{
    similarityScore->companyScores.scores = NULL;
    similarityScore->companyScores.maxId = INT32_MIN;
    similarityScore->companyScores.scoresCount = 0;
    similarityScore->industryScores.scores = NULL;
    similarityScore->industryScores.maxId = INT32_MIN;
    similarityScore->industryScores.scoresCount = 0;
    similarityScore->titleScores.scores = NULL;
    similarityScore->titleScores.maxId = INT32_MIN;
    similarityScore->titleScores.scoresCount = 0;
}

void freeSimilarityScore(struct SimilarityScores *similarityScore)
{
    free(similarityScore->companyScores.scores);
    similarityScore->companyScores.scores = NULL;
    similarityScore->companyScores.maxId = INT32_MIN;
    free(similarityScore->industryScores.scores);
    similarityScore->industryScores.scores = NULL;
    similarityScore->industryScores.maxId = INT32_MIN;
    free(similarityScore->titleScores.scores);
    similarityScore->titleScores.scores = NULL;
    similarityScore->titleScores.maxId = INT32_MIN;
}

bool marshallSimilarityScores(const SimilarityScores *similarityScore, struct json_object *addTo)
{
    if (similarityScore == NULL || addTo == NULL)
    {
        return false;
    }

    struct json_object *arrObj = json_object_new_array_ext(similarityScore->companyScores.scoresCount);
    if (arrObj == NULL)
    {
        return false;
    }

    if (similarityScore->companyScores.scoresCount > 0)
    {
        for (size_t i = 0; i <= similarityScore->companyScores.maxId; i++)
        {
            if (similarityScore->companyScores.scores[i] == 0)
            {
                continue;
            }
            struct json_object *obj = json_object_new_object();
            if (obj != NULL)
            {
                struct json_object *obj_key = json_object_new_int(i);
                json_object_object_add(obj, id_Key, obj_key);
                struct json_object *obj_val = json_object_new_int((int32_t)(similarityScore->companyScores.scores[i]));
                json_object_object_add(obj, value_Key, obj_val);
            }
            json_object_array_add(arrObj, obj);
        }
    }

    int addResult = json_object_object_add(addTo, similarCompaniesScores_Key, arrObj);
    if (addResult != 0)
    {
        return false;
    }

    arrObj = json_object_new_array_ext(similarityScore->industryScores.scoresCount);
    if (arrObj == NULL)
    {
        return false;
    }

    if (similarityScore->industryScores.scoresCount > 0)
    {
        for (size_t i = 0; i <= similarityScore->industryScores.maxId; i++)
        {
            if (similarityScore->industryScores.scores[i] == 0)
            {
                continue;
            }
            struct json_object *obj = json_object_new_object();
            if (obj != NULL)
            {
                struct json_object *obj_key = json_object_new_int(i);
                json_object_object_add(obj, id_Key, obj_key);
                struct json_object *obj_val = json_object_new_int((int32_t)(similarityScore->industryScores.scores[i]));
                json_object_object_add(obj, value_Key, obj_val);
            }
            json_object_array_add(arrObj, obj);
        }
    }

    addResult = json_object_object_add(addTo, similarIndustryScores_Key, arrObj);
    if (addResult != 0)
    {
        return false;
    }

    return true;
}

const float GetCompanySimilarityScore(const int32_t companyId, const SimilarityScores *similarityScore)
{
    if (companyId > similarityScore->companyScores.maxId)
    {
        return 0.0f;
    }
    return (float)similarityScore->companyScores.scores[companyId] / (float)MAX_SCORE_VALUE_ACCEPTED;
}

const float GetIndustrySimilarityScore(const int32_t industryId, const SimilarityScores *similarityScore)
{
    if (industryId > similarityScore->industryScores.maxId)
    {
        return 0.0f;
    }
    return (float)similarityScore->industryScores.scores[industryId] / (float)MAX_SCORE_VALUE_ACCEPTED;
}

const float GetTitleSimilarityScore(int32_t profileId, const int32_t titleId, const SimilarityScores* similarityScore)
{
    if(titleId == 0)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: titleId == 0 for profile id %d", profileId);
        return 0.0f;
    }

    if(titleId > similarityScore->titleScores.maxId)
    {
        return 0.0f;
    }

    return (float)similarityScore->titleScores.scores[titleId] / (float)MAX_SCORE_VALUE_ACCEPTED;
}

static inline size_t parseBinarySimilarityScores(SimilarityScoreFlatMap *similarityMap, const NetworkSimilarityScoreHeader *data,
                                                 const int32_t dataSize, const int32_t dataCount)
{
    // sanity check
    if (dataCount * sizeof(NetworkSimilarityScoreData) > dataSize)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Expected similarity score element size %zu. "
                                       "We have %d bytes and expected %d for %d elements",
                    sizeof(NetworkSimilarityScoreData),
                    dataSize, dataCount * sizeof(NetworkSimilarityScoreData), dataCount);
        return 0;
    }

    // Let's not process too much if we have no data to process
    if (dataCount == 0)
    {
        similarityMap->scores = malloc(1 * sizeof(similarityMap->scores[0]));
        return sizeof(NetworkSimilarityScoreData);
    }

    // get the max ID from this array
    int32_t maxIdAtIndex = dataCount - 1;
    int32_t maxId = data->data[maxIdAtIndex].id;
    while (maxId > MAX_ID_VALUE_ACCEPTED_FROM_NETWORK && maxIdAtIndex > 0)
    {
        maxIdAtIndex--;
        maxId = data->data[maxIdAtIndex].id;
    }
    if (maxIdAtIndex != dataCount - 1)
    {
        int32_t IdsThrownAway = dataCount - 1 - maxIdAtIndex;
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Similarity score ignored %d IDs that are greater than max allowed %d!",
                    IdsThrownAway, MAX_ID_VALUE_ACCEPTED_FROM_NETWORK);
    }
    if (maxIdAtIndex > 0 && data->data[maxIdAtIndex - 1].id >= maxId)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Was expecting similarity scores array to be ordered!");
    }

    // we did not manage to get a valid ID. Maybe data is bad
    if (maxId > MAX_ID_VALUE_ACCEPTED_FROM_NETWORK)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Could not find any IDs smaller than %d. ",
                    MAX_ID_VALUE_ACCEPTED_FROM_NETWORK);
        return 0;
    }

    DSSP_LOG_MESSAGE(INFO_LOG_MSG, "Max Id is : %d", maxId);

    // in case you sent JSON data also, than maxId will not be 0. When JSON support gets dropped, this should always be 0
    if (maxId > similarityMap->maxId)
    {
        similarityMap->maxId = maxId;
    }
    size_t bytesToAlloc = (similarityMap->maxId + 1) * sizeof(data->data[0].score);
    similarityMap->scores = realloc(similarityMap->scores, bytesToAlloc);
    similarityMap->scoresCount = dataCount;

    if (similarityMap->scores == NULL)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Failed to allocate %zu bytes for similarity score map", bytesToAlloc);
        // consider this block parsed. Though at this point the application would crash somewhere
        return sizeof(NetworkSimilarityScoreHeader) + dataCount * sizeof(NetworkSimilarityScoreData);
    }

    memset(similarityMap->scores, 0, bytesToAlloc); // every score by default is 0

    // iterate through the blocks and insert them into the tree
    for (size_t index = 0; index < dataCount; index++)
    {
        int32_t networkId = (data->data[index].id);

        if (networkId > maxId)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: At index %zu, id %d is unexpectedly large(not ordered?). Max allowed %d. Score %d", index, networkId, maxId, data->data[index].score);
            continue;
        }
        int32_t networkScore = data->data[index].score;
        if (networkScore == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: invalid 0 value at index %zu, for id %d. Ignoring it", index, networkId);
            continue;
        }

        if (networkScore > MAX_SCORE_VALUE_ACCEPTED)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: value (%d) at index %zu, for id %d is larger than maximum accepted (%d). Ignoring it",
                        networkScore, index, networkId, MAX_SCORE_VALUE_ACCEPTED);
            continue;
        }
        //        DSSP_LOG_MESSAGE(INFO_LOG_MSG, "adding similarity score id %d, val %d", networkId, data->data[index].score);
        if (similarityMap->scores[networkId] != 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: At index %zu, id %d was already set to %d score."
                                           "Updating it to %d",
                        index, networkId, similarityMap->scores[networkId], networkScore);
        }
        similarityMap->scores[networkId] = networkScore;
    }
    LOG_MESSAGE(DEBUG_LOG_MSG, "Debug: Added %d similarity scores. Max id %d", dataCount, maxId);

    return sizeof(NetworkSimilarityScoreHeader) + dataCount * sizeof(NetworkSimilarityScoreData); // should be the same as data->size
}

bool unmarshallSimilarityScoresBinary(SimilarityScores *similarityScore, const uint8_t *networkPacket, int32_t bytesRemain)
{
    DSSP_LOG_MESSAGE(INFO_LOG_MSG, "Checking if there are binary similarity scores. Bytes available : %zd", bytesRemain);
    while (bytesRemain >= sizeof(NetworkSimilarityScoreHeader))
    {
        // reinterpret data
        const NetworkSimilarityScoreHeader *data = (const NetworkSimilarityScoreHeader *)networkPacket;
        unsigned int dataSize = (data->size);
        unsigned int dataType = (data->type);
        unsigned int dataCount = (data->blockCount);

        if (dataType >= NDBT_MAX_VALUE)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Unknown similarity score type %d. Max known is %d. ", dataType, NDBT_MAX_VALUE);
        }

        if (dataSize > bytesRemain)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: Network packet is too small. Received %zu bytes. Current "
                                           "block header says, it needs %d bytes. Similarity score type %d",
                        bytesRemain, dataSize, dataType);
            return false;
        }

        DSSP_LOG_MESSAGE(INFO_LOG_MSG, "Debug : Parsing new data block. Type %d, blocks %d, size %d", dataType, dataCount, dataSize);

        SimilarityScoreFlatMap *similarityMap;
        // call assigned handler
        switch (dataType)
        {
        case NDBT_COMPANY_SIMILARITY_SCORE:
        {
            similarityMap = &similarityScore->companyScores;
        }
        break;
        case NDBT_INDUSTRY_SIMILARITY_SCORE:
        {
            similarityMap = &similarityScore->industryScores;
        }
        break;
        case NDBT_TITLE_SIMILARITY_SCORE:
        {
            similarityMap = &similarityScore->titleScores;
        }
        break;
        default:
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Warning: Unknown similarity score block type. Block type %d, size %d, elements %d, bytes avail %d",
                        dataType, dataSize, dataCount, bytesRemain);
            // maybe incorrect format while testing. Unexpected situation, breaking out of it
            return true;
        }
        break;
        }
        // actually parse the data block
        size_t bytesConsumed = parseBinarySimilarityScores(similarityMap, data, dataSize, dataCount);
        if (bytesConsumed == 0)
        {
            LOG_MESSAGE(ATTENTION_LOG_MSG, "Warning: Unexpected 0 bytes parsed. Aboting further parsing. Block type %d, size %d, elements %d, bytes avail %d",
                        dataType, dataSize, dataCount, bytesRemain);
            return false;
        }
        // based on the parser, advance netowrk packet pointer
        bytesRemain -= bytesConsumed;
        networkPacket += bytesConsumed;
    }
    return true;
}

void addScoreToMap(SimilarityScoreFlatMap *scoreMap, const int32_t entryId, const unsigned short score)
{
    DSSP_LOG_MESSAGE(INFO_LOG_MSG, "adding similarity score id %d, val %d", entryId, score);
    if (entryId >= MAX_ID_VALUE_ACCEPTED_FROM_NETWORK)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: Id %d is larger than maximum accepted (%d).", entryId, MAX_ID_VALUE_ACCEPTED_FROM_NETWORK);
        return;
    }
    if (scoreMap->maxId < entryId)
    {
        DSSP_LOG_MESSAGE(INFO_LOG_MSG, "reallocating buffer from old size %d to %d", scoreMap->maxId, entryId);
        size_t bytesToAlloc = (entryId + 1) * sizeof(scoreMap->scores[0]);
        scoreMap->scores = realloc(scoreMap->scores, bytesToAlloc);
        if (scoreMap->maxId <= 0)
        {
            // first time init
            memset(scoreMap->scores, 0, bytesToAlloc);
        }
        else
        {
            // do not overwrite previous max Id
            memset(&scoreMap->scores[scoreMap->maxId + 1], 0, (entryId + 1 - (scoreMap->maxId + 1)) * sizeof(scoreMap->scores[0]));
        }
        scoreMap->maxId = entryId;
    }
    if (scoreMap->scores[entryId] == 0)
    {
        scoreMap->scoresCount++;
        DSSP_LOG_MESSAGE(INFO_LOG_MSG, "Id %d is new, creating value %d. Total values %d", entryId, score, scoreMap->scoresCount);
    }
    if (score > MAX_SCORE_VALUE_ACCEPTED)
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Attention: Id %d is larger than maximum accepted (%d)."
                                       "score %d found. Ignoring it",
                    score, MAX_SCORE_VALUE_ACCEPTED, entryId);
        return;
    }
    scoreMap->scores[entryId] = score;
}
