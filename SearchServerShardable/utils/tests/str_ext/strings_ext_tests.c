#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <strings_ext.h>

#define MAX_STR_LEN 1000
#define MAX_RESULT_COUNT 1000

void TestStrPartialDup()
{
    printf("==========================\n");
    printf("Start StrPartialStrDup test\n");
    const char* in1 = "This is a test string";
    char *res = StrPartialStrDup(in1, 5, 9);
    if(res == NULL)
    {
        printf("Error in StrPartialStrDup\n");
    }
    printf("Input string is : %s\n", in1);
    printf("Result string is : %s\n", res);
    free(res);
    printf("==========================\n");
}

void TestStrSplit()
{
    char* results[MAX_RESULT_COUNT];
    size_t res_count;

    printf("==========================\n");
    printf("Start string split test\n");
    // Do a normal test
    const char* t1 = "This#,#is#,#a#,#test";
    printf("Input string is : %s\n", t1);
    StrSplit(t1, "#,#", (char**)&results, &res_count, MAX_RESULT_COUNT);
    for (size_t i = 0; i < res_count; i++)
    {
        printf("%s\n", results[i]);
    }
    StrSplitCleanup(results, res_count);

    // check a reuse of the variable to see if it bugs out
    const char* t2 = "another#,#test#,##,#";
    printf("Input string is : %s\n", t2);
    StrSplit(t2, "#,#", (char**)&results, &res_count, MAX_RESULT_COUNT);
    for (size_t i = 0; i < res_count; i++)
    {
        printf("%s\n", results[i]);
    }
    StrSplitCleanup(results, res_count);

    // a few sanity tests
    StrSplit(NULL, "test", (char**)&results, &res_count, MAX_RESULT_COUNT);
    StrSplit("bla", NULL, (char**)&results, &res_count, MAX_RESULT_COUNT);
    StrSplit("bla", "a", NULL, &res_count, MAX_RESULT_COUNT);
    StrSplitCleanup(results, res_count);
    StrSplit(t1, "#,#", (char**)&results, &res_count, 0);
    StrSplitCleanup(results, res_count);
    StrSplit(t1, "#,#", (char**)&results, &res_count, -1);
    StrSplitCleanup(results, res_count);
    StrSplit(t1, "#,#", (char**)&results, NULL, 0);
    StrSplitCleanup(results, res_count);
    StrSplit("#,#", "#,#", (char**)&results, NULL, 0);
    StrSplitCleanup(results, res_count);
    printf("==========================\n");
}

void TestStringCharReplace()
{
    printf("==========================\n");
    printf("Start char remove from string test\n");
    char tempStr[MAX_STR_LEN];
    const char* in1 = "This \" is a test \\ \" input string";
    int removeRes = StrRemoveChar(in1, '\"', tempStr, MAX_STR_LEN);
    if (removeRes != 0)
    {
        printf("Could not remove chars from input\n");
    }
    printf("Input string is : %s\n", in1);
    printf("Result string is : %s\n", tempStr);

    // sanity tests
    StrRemoveChar(NULL, '\"', tempStr, MAX_STR_LEN);
    StrRemoveChar(in1, '\0', tempStr, MAX_STR_LEN);
    StrRemoveChar(in1, ' ', NULL, MAX_STR_LEN);
    StrRemoveChar(in1, ' ', tempStr, 0);
    StrRemoveChar(in1, ' ', tempStr, 1);
    printf("==========================\n");
}

void TestStringQuote()
{
    printf("==========================\n");
    printf("check if string is quote enclosed\n");
    const char* in1 = "\"is enclosed\"";
    const char* in2 = "not enclosed";
    printf("Result : %d Input : %s\n", StrIsQuoteEnclosed(in1), in1);
    printf("Result : %d Input : %s\n", StrIsQuoteEnclosed(in2), in2);
    printf("==========================\n");
}

void TestStringRemoveChar()
{
    printf("==========================\n");
    printf("test remove character from string\n");
    char tempStr[MAX_STR_LEN];
    const char* in1 = "This \" is a test \\ \" input string";
    int removeRes = StrRemoveChar(in1, '\"', tempStr, MAX_STR_LEN);
    if (removeRes != 0)
    {
        printf("Could not remove chars from input\n");
    }
    printf("Input string is : %s\n", in1);
    printf("Result string is : %s\n", tempStr);
    printf("==========================\n");
}

void TestStringCompareStore()
{
    printf("==========================\n");
    printf("test string store for multi compares \n");
    SearchedString a;
    DBString b;

    initSearchedString(&a,"\"Test\"",SCSSDV_DUPLICATE_INPUT, SCSGWV_SPLIT_WORDS);
    initDBString(&b, "test", 1);
    printf("Input string 1 : %s\n", a.strOriginal.str);
    printf("Input string 2 : %s\n", b.str);
    printf("Result string compare is equal: %d\n", IsSearchiStringInDbString(&a,&b, SSDBSWOV_DO_NOT_KEEP_WORD_ORDER));
    freeSearchedString(&a);
    freeDBString(&b);

    initSearchedString(&a, "joh", SCSSDV_DUPLICATE_INPUT, SCSGWV_SPLIT_WORDS);
    initDBString(&b, "John Doe", 1);
    printf("Input string 1 : %s\n", a.strOriginal.str);
    printf("Input string 2 : %s\n", b.str);
    printf("Result string compare is equal: %d\n", IsSearchiStringInDbString(&a, &b, SSDBSWOV_DO_NOT_KEEP_WORD_ORDER));
    freeSearchedString(&a);
    freeDBString(&b);

    initSearchedString(&a, "a B c", SCSSDV_DUPLICATE_INPUT, SCSGWV_SPLIT_WORDS);
    initDBString(&b, "C b A", 1);
    printf("Input string 1 : %s\n", a.strOriginal.str);
    printf("Input string 2 : %s\n", b.str);
    printf("Result string compare is equal: %d\n", IsSearchiStringInDbString(&a, &b, SSDBSWOV_DO_NOT_KEEP_WORD_ORDER));
    freeSearchedString(&a);
    freeDBString(&b);

    initSearchedString(&a, "a cto c", SCSSDV_DUPLICATE_INPUT, SCSGWV_SPLIT_WORDS);
    initDBString(&b, "C chief technology officer cfo A D", 1);
    printf("Input string 1 : %s\n", a.strOriginal.str);
    printf("Input string 2 : %s\n", b.str);
    printf("Result string compare is equal: %d\n", IsSearchiStringInDbString(&a, &b, SSDBSWOV_DO_NOT_KEEP_WORD_ORDER));
    freeSearchedString(&a);
    freeDBString(&b);

    initSearchedString(&a, "\"\"", SCSSDV_DUPLICATE_INPUT, SCSGWV_SPLIT_WORDS);
    printf("Input string : %s\n", a.strOriginal.str);
    freeSearchedString(&a);

    initSearchedString(&a, NULL, SCSSDV_DUPLICATE_INPUT, SCSGWV_SPLIT_WORDS);
    printf("Input string : %s\n", a.strOriginal.str);
    freeSearchedString(&a);

    char* NoDupStr = StrPartialStrDup("dupe test", 0, 8);
    initSearchedString(&a, NoDupStr, SCSSDV_TAKE_INPUT_OWNERSHIP, SCSGWV_SPLIT_WORDS);
    printf("Input string : %s\n", a.strOriginal.str);
    freeSearchedString(&a);

    NoDupStr = StrPartialStrDup("\"dupe 2 test\"", 0, 13);
    initSearchedString(&a, NoDupStr, SCSSDV_TAKE_INPUT_OWNERSHIP, SCSGWV_SPLIT_WORDS);
    printf("Input string : %s\n", a.strOriginal.str);
    freeSearchedString(&a);

    printf("==========================\n");
}

void TestStringCompareStoreWithSplit()
{
    printf("==========================\n");
    printf("test StrStringVectToStringStore \n");
    const char* in[2];
    in[0] = "John Doe ";
    in[1] = " of the mighty  ";

    DBString res;
    int converErr = StrStringVectToStringStore((char **)in, 2, (const char*)" ", &res);
    if (converErr != 0)
    {
        printf("Could not convert input. Should not happen\n");
    }

    printf("Printing list of words : ");
    printf("%s\n", res.str);
    freeDBString(&res);

    printf("==========================\n");
}

void TestTrim()
{
    printf("==========================\n");
    printf("test StrTrim \n");
    char* res;
    res = StrTrim(" one in front");
    if (res)
    {
        printf("'%s'\n", res);
        free(res);
    }
    res = StrTrim("one in back ");
    if (res)
    {
        printf("'%s'\n", res);
        free(res);
    }
    res = StrTrim(" one one ");
    if (res)
    {
        printf("'%s'\n", res);
        free(res);
    }
    res = StrTrim("none");
    if (res)
    {
        printf("'%s'\n", res);
        free(res);
    }
    res = StrTrim(" ");
    if (res)
    {
        printf("'%s'\n", res);
        free(res);
    }
    res = StrTrim(NULL);
    if (res)
    {
        printf("'%s'\n", res);
        free(res);
    }

    printf("==========================\n");
}

void TestStriStr()
{
    printf("==========================\n");
    printf("test stristr \n");
#define Test(a,b) res = stristr(a, b); printf("'%s' was searched in '%s'. Result : %d\n", b, a, (res != NULL));
    char* res;
    Test("long string", "G s");
    Test("lonG string", "g S");

    printf("==========================\n");
}

void TestRemovePunctuations()
{
    printf("==========================\n");
    printf("test StrRemovePuntuation \n");
    char* res;
#define TestPunct(a) res = StrRemovePunctuation(a, 0); printf("input '%s' Result : %s\n", a, res); free(res); res = NULL;
    TestPunct("one day.");
    TestPunct("-minus");
    TestPunct("\tboth\n");
    TestPunct("(tr)i/");
    TestPunct(":");
    TestPunct("-:/");
    TestPunct("");
    TestPunct("nothing");

    printf("==========================\n");
}

int main(int argc, char* argv[])
{
    TestStrPartialDup();
    TestStrSplit();
    TestStringCharReplace();
    TestStringQuote();
    TestStringRemoveChar();
    TestStringCompareStore();
    TestStringCompareStoreWithSplit();
    TestTrim();
    TestStriStr();
    TestRemovePunctuations();
    return 0;
}