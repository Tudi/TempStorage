#include <search_criteria.h>
#include <similarity_score.h>
#include <utils.h>
#include <json_specialized_array.h>
#include <json_utils.h>
#include <stdlib.h>

static const char* role_Key           = "role_code";
static const char* localities_Key     = "localities";
static const char* filters_Key        = "filters";
static const char* organizationID_Key = "organization_id";

void initSearchCriteria(struct SearchCriteria* searchCriteria)
{
    searchCriteria->role      = 0;
    kv_init(searchCriteria->localities);
    kv_init(searchCriteria->filters);
    initSimilarityScore(&searchCriteria->similarityScores);
    searchCriteria->organizationID = 0;
    initBitField(searchCriteria->bfLocalities);
}

void freeSearchCriteria(struct SearchCriteria* searchCriteria)
{
    kv_destroy(searchCriteria->localities);
    kv_init(searchCriteria->localities);

    size_t i = 0;
    for(i = 0; i < kv_size(searchCriteria->filters); ++i) {
        freeSearchFilter(&kv_A(searchCriteria->filters, i));
    }
    kv_destroy(searchCriteria->filters);
    kv_init(searchCriteria->filters);
    freeSimilarityScore(&searchCriteria->similarityScores);
    freeBitField(searchCriteria->bfLocalities);
}

struct json_object* marshallSearchCriteria(const struct SearchCriteria* searchCriteria)
{
    struct json_object* obj = json_object_new_object();
    if(obj != NULL)
    {
        json_object_object_add(obj, role_Key, json_object_new_int(searchCriteria->role));
        JSON_ADD_INT32_ARRAY(obj, localities_Key , searchCriteria->localities);
        JSON_ADD_ARRAY_ADDR(marshallSearchFilter, obj, filters_Key, searchCriteria->filters);
        marshallSimilarityScores(&searchCriteria->similarityScores, obj);
        json_object_object_add(obj, organizationID_Key, json_object_new_int(searchCriteria->organizationID));
    }

    return obj;
}

/// <summary>
/// Convert array of int into a bitfield. This is a speed optimization.
/// Function is not static because it's getting called from testing
/// </summary>
/// <param name="searchCriteria"></param>
void convertLocalitiesToBitfield(struct SearchCriteria* searchCriteria)
{
    // initialize localities lookupTable. 
    int32_t maxLocalityId = 0;
    // In V2 replace this constant value with actual max locality ID available from profiles
#define CONST_MAX_LOCALITY_ID_AVAILABLE 1000000 
    for (size_t localityIndex = 0; localityIndex < kv_size(searchCriteria->localities); localityIndex++)
    {
        int32_t curLocalityId = kv_A(searchCriteria->localities, localityIndex);
        // there is no reason to allocate memory for IDs that we are sure they won't be used
        if (curLocalityId > CONST_MAX_LOCALITY_ID_AVAILABLE)
        {
            // This warning is not required. We simply do not have profiles with this large locality ID
            // There is no point allocating memory for something we know for 100% it will not be used
            LOG_MESSAGE(INFO_LOG_MSG, "Warning! Unexpectedly large locality ID %d", curLocalityId);
            continue;
        }
        // sanity check to avoid negative indexing in memory. This should never happen
        if (curLocalityId <= 0)
        {
            LOG_MESSAGE(INFO_LOG_MSG, "Warning! Invalid requested locality ID %d", curLocalityId);
            continue;
        }
        if (curLocalityId > maxLocalityId)
        {
            maxLocalityId = curLocalityId;
        }
    }
    if (maxLocalityId > 0)
    {
        BitFieldResize(searchCriteria->bfLocalities, maxLocalityId);
        if (searchCriteria->bfLocalities.field == NULL)
        {
            LOG_MESSAGE(CRITICAL_LOG_MSG, "Error ! Could not allocate %d memory. Search will not provide results !", searchCriteria->bfLocalities.size);
            searchCriteria->bfLocalities.size = 0;
        }
        else
        {
            for (size_t localityIndex = 0; localityIndex < kv_size(searchCriteria->localities); localityIndex++)
            {
                int32_t curLocalityId = kv_A(searchCriteria->localities, localityIndex);
                if (curLocalityId < CONST_MAX_LOCALITY_ID_AVAILABLE && curLocalityId > 0)
                {
                    BitFieldSet(searchCriteria->bfLocalities, curLocalityId);
                }
            }
        }
    }
}

bool unmarshallSearchCriteria(struct SearchCriteria* searchCriteria, const struct json_object* obj,
    const uint8_t* networkPacket, int32_t networkBytes)
{
    freeSearchCriteria(searchCriteria);
    initSearchCriteria(searchCriteria);

    bool b1 = jsonGetInt32(obj, role_Key, &searchCriteria->role);

    bool b2 = false;
    JSON_GET_INT32_ARRAY(obj, localities_Key, searchCriteria->localities, b2);

    bool b3 = false;
    JSON_GET_SEARCHFILTER_ARRAY(obj, filters_Key, searchCriteria->filters, b3);

    bool b4 = jsonGetInt32(obj, organizationID_Key, &searchCriteria->organizationID);

    if(!(b1 && b2 && b3 && b4))
    {
        LOG_MESSAGE(ATTENTION_LOG_MSG, "Error: unmarshallSearchCriteria() failed.");
        return false;
    }

    // JSON format similarity scores will be depracated, keeping it due to easy testing
    bool b7 = unmarshallSimilarityScoresBinary(&searchCriteria->similarityScores, networkPacket, networkBytes);
    if(b7 == false)
    { 
        return false; 
    }

    convertLocalitiesToBitfield(searchCriteria);

    return true;
}
