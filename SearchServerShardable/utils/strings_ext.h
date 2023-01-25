#ifndef _STRINGS_EXT_H_
#define _STRINGS_EXT_H_

// for sanity checks. To avoid strange value processing
#define MAX_SPLIT_RESULTS 1000
#ifndef CHECK_AND_FREE_STRINGSTORE
    #define CHECK_AND_FREE_DBSTRING_P(val) if(val != NULL) { freeStringCompareStore(val); free(val); val = NULL; }
#endif

typedef struct StringDescriptorExtendedStruct
{
//    unsigned short len;
    char* str;
}StringDescriptorExtendedStruct;

// if memory constraint is really high, this structure could be simplified to a minimal : "char *" + "isQuoteEnclosed"
typedef struct SearchedString
{
    char isQuoteEnclosed; // If the original string was enclosed in quotes. Required for exact match test.
    StringDescriptorExtendedStruct strOriginal; // ! quotes got removed + lower case + StrStandardizeTitle !
    size_t wordCount; // number of words and their descriptors
    // future feature : The numbers should be ascending ordered so we can exit the search as soon as possible
    // in the future profile to see if it's better to store word start indexes instead pointers. In theory it should be better
    StringDescriptorExtendedStruct *wordDescriptors; 
}SearchedString;

typedef struct DBString
{
//    unsigned short len;
    char* str;
}DBString;

/// <summary>
/// Prepare a string that will be used for many comparisons. String will be lower cased + quotes removed + standardized
/// </summary>
/// <param name="scs">structure that will get initialized</param>
/// <param name="str">Use this string to initialize the structure</param>
/// <param name="strdup">0 mean structure should take ownership of the string instead duplicating input string</param>
/// <param name="generateWords">only searched strings should be split to words</param>
void initSearchedString(SearchedString* scs, char *str);
void freeSearchedString(SearchedString* scs);

void initDBString(DBString* scs, char* str);
void freeDBString(DBString* scs);

/// <summary>
/// Compare 2 strings if they are equal. Strings are stored lower case + standardized
/// </summary>
/// <param name="scs1">String 1</param>
/// <param name="scs2">String 2</param>
/// <returns></returns>
int IsSearchiStringInDbString(const SearchedString* searchedStr, const DBString* databaseStr);

/// <summary>
/// strdup a part of the input string. Adds terminator 0 to the end. Does not check out of bounds copy !
/// </summary>
/// <param name="source">input string</param>
/// <param name="start">character index where the copy should start</param>
/// <param name="charCount">Number of characters that will be copied from source</param>
/// <returns>substring or NULL on error</returns>
char* StrPartialStrDup(const char* source, const int start, const int charCount);

/// <summary>
/// Split the input string into the result store. The result will need to be destroyed
/// </summary>
/// <param name="source">Input string</param>
/// <param name="delim">Delimitator string</param>
/// <param name="res">array of character pointers. string will be allocated and placed as poitners</param>
/// <param name="resCount">number of substrings that are returned</param>
/// <param name="maxRes">Maximum number of substrings the "res" param can store</param>
/// <returns>error code</returns>
int StrSplit(const char* source, const char* delim, char** res, size_t* resCount, const size_t maxRes);

/// <summary>
/// Convert string to lower case, in place
/// </summary>
/// <param name="str">Input string that will be updated</param>
void StrToLower(char* str);

/// <summary>
/// Remove spaces from the start and the end of the string
/// Remove double ' ' from the string. Avoids empty words to be created on split
/// This is an inplace operation. Does not duplicate the string !
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
void StrTrimScoringClient(char* str);

/// <summary>
/// Project specific function to extend abreviations to full values. This works inside a string where the title could be only part of the full string
/// </summary>
/// <param name="str"></param>
/// <returns>expanded string</returns>
char* StrStandardizeTitle(char* str);

/// <summary>
/// In place remove punctuation characters. If anything is found, the string becomes shorter
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
void StrRemovePunctuation(char* str);

/// <summary>
/// !! takes ownership ower input string. Will alter it and possibly free it !!
/// Applies changes to the string :
/// - allocates new buffer for the string ( strdup )
/// - convert to lowercase
/// - trim
/// - converts compacted titles to expanded version
/// - remove punctuations
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
char* StrStandardizeScoringClient(char* str);

/// <summary>
/// Replace a string with a smaller string
/// Inplace operation
/// </summary>
/// <param name="largeStr"></param>
/// <param name="substr"></param>
/// <param name="replace"></param>
/// <returns>changes made</returns>
int StrReplaceAllToSmaller(char* largeStr, const char* substr, const char* replace);

#endif