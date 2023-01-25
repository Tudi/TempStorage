#include <similarity_score.h>
#include <json_array.h>
#include <json_object.h>
#include <json_tokener.h>
#include <stdint.h>
#include <stdarg.h>
#include <setjmp.h>
#include <cmocka.h>
#include <stdio.h>
#include <arpa/inet.h>

SimilarityScores scores;

int test_SimilarityScore_setUp(void** state)
{
    initSimilarityScore(&scores);
    return 0;
}

int test_SimilarityScore_tearDown(void** state)
{
    freeSimilarityScore(&scores);
    return 0;
}

void test_marshall_unmarshallBinary_SimilarityScore_succeeds(void** state)
{
    // I know this is not required, but I'm testing with file write also
    const char* searchString = "{ \"role_code\": 56, \"localities\": [], \"filters\": [ { \"filter\": \"current_previous_title_include\", "
        "\"text_value\": \"Supply chain\", \"code_value\": \"\", \"range_low\": 0, \"range_high\": 0, \"modifier\": \"\" } ], "
        "\"organization_id\":5, \"nationwide\": true }";
    size_t inputLen = strlen(searchString);

    const int companyBlocks = 5;
    size_t totalNetworkData = inputLen + 1 + sizeof(NetworkSimilarityScoreHeader) + companyBlocks * sizeof(NetworkSimilarityScoreData);
    // add some industry data also
    const int industryBlocks = 9;
    totalNetworkData += sizeof(NetworkSimilarityScoreHeader) + industryBlocks * sizeof(NetworkSimilarityScoreData);

    uint8_t* searchStringBinary = malloc(totalNetworkData);
    assert_non_null(searchStringBinary);
    strcpy((char*)searchStringBinary, searchString);

    NetworkSimilarityScoreHeader* header = (NetworkSimilarityScoreHeader*)&searchStringBinary[inputLen + 1];
    header->blockCount = (companyBlocks);
    header->type = (NDBT_COMPANY_SIMILARITY_SCORE);
    header->size = (sizeof(NetworkSimilarityScoreHeader) + companyBlocks * sizeof(NetworkSimilarityScoreData));
    header->data[0].id = (84); // used while testing profile id 3 search
    header->data[0].score = 500;
    header->data[1].id = (9007); // used while testing profile id 3 search
    header->data[1].score = 8000;
    header->data[2].id = (32860); // used while testing profile id 2 search
    header->data[2].score = 5000;
    header->data[3].id = (110413); // used while testing profile id 2 search
    header->data[3].score = 6000;
    header->data[4].id = (4102710); // used while testing profile id 2 search
    header->data[4].score = 7000;

    // industry stuff
    header = (NetworkSimilarityScoreHeader*)((char*)header + (header->size));
    header->blockCount = (industryBlocks);
    header->type = (NDBT_INDUSTRY_SIMILARITY_SCORE);
    header->size = (sizeof(NetworkSimilarityScoreHeader) + industryBlocks * sizeof(NetworkSimilarityScoreData));
    header->data[0].id = (4);
    header->data[0].score = 3000;
    header->data[1].id = (6);
    header->data[1].score = 2000;
    header->data[2].id = (5); // profile 3, company 9007
    header->data[2].score = 3000;
    header->data[3].id = (35);// profile 3, company 9007
    header->data[3].score = 3500;
    header->data[4].id = (45);// profile 3, company 9007
    header->data[4].score = 4000;
    header->data[5].id = (78);// profile 3, company 9007
    header->data[5].score = 4500;
    header->data[6].id = (93);// profile 3, company 9007
    header->data[6].score = 5000;
    header->data[7].id = (142);// profile 3, company 9007
    header->data[7].score = 5500;
    header->data[8].id = (189);// profile 3, company 9007
    header->data[8].score = 2000;

//    FILE* f = fopen("search.bin", "wb"); fwrite(searchStringBinary, 1, totalNetworkData, f); fclose(f);

    // parse similarity score sections
    bool ret = unmarshallSimilarityScoresBinary(&scores, &searchStringBinary[inputLen + 1], totalNetworkData - (inputLen + 1));

    assert_true(ret);

    // check if we can find a value inside 
    float val;
    val = GetCompanySimilarityScore(32860, &scores);
    assert_int_equal((int)(val* MAX_SCORE_VALUE_ACCEPTED), 5000);

    val = GetCompanySimilarityScore(110413, &scores);
    assert_int_equal((int)(val * MAX_SCORE_VALUE_ACCEPTED), 6000);

    val = GetCompanySimilarityScore(4102710, &scores);
    assert_int_equal((int)(val * MAX_SCORE_VALUE_ACCEPTED), 7000);

    val = GetCompanySimilarityScore(4, &scores);
    assert_float_equal(val, 0.0, 4);


    val = GetIndustrySimilarityScore(4, &scores);
    assert_float_equal(val, 0.3, 4);

    val = GetIndustrySimilarityScore(6, &scores);
    assert_float_equal(val, 0.2, 4);

    val = GetIndustrySimilarityScore(7, &scores);
    assert_float_equal(val, 0.0, 4);

    free(searchStringBinary);
}
