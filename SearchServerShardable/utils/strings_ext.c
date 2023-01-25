#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include "utils.h"
#include "strings_ext.h"

static inline int StrIsQuoteEnclosed(const char* str);

/// <summary>
/// Descriptor for a string that will be compared multiple times
/// </summary>
/// <param name="desc">Structure where to store the data</param>
/// <param name="str">string that will be stored inside the descriptor</param>
/// <param name="doStrDup">Can we take ownership of this string</param>
void initStringDescriptorExtendedStruct(StringDescriptorExtendedStruct* desc, char* str)
{
    if (desc == NULL)
    {
        return;
    }

    if (str != NULL)
    {
        size_t len = strlen(str);
        desc->str = (char*)malloc(len + 1);
        memcpy(desc->str, str, len + 1);
    }
    else
    {
        memset(desc, 0, sizeof(StringDescriptorExtendedStruct));
    }
}

void freeStringDescriptorExtendedStruct(StringDescriptorExtendedStruct* desc)
{
    if (desc == NULL)
    {
        return;
    }
    free(desc->str);
    desc->str = NULL;
}


/// <summary>
/// Prepare a string that will be used for many comparisons. String will be lower cased + quotes removed + standardized
/// </summary>
/// <param name="scs">structure that will get initialized</param>
/// <param name="str">Use this string to initialize the structure</param>
/// <param name="strdup">0 mean structure should take ownership of the string instead duplicating input string</param>
void initSearchedString(SearchedString* scs, char* str)
{
    if (scs == NULL)
    {
        return;
    }

    if (str != NULL)
    {
        char* oldLocalStr = NULL;
        char* localStr = strdup(str); // clone it

        scs->isQuoteEnclosed = StrIsQuoteEnclosed(localStr);
        if (scs->isQuoteEnclosed == 1)
        {
            // create a new string that is not quote enclosed
            size_t oldLen = strlen(localStr);
            oldLocalStr = localStr;
            localStr = StrPartialStrDup(localStr, 1, oldLen - 2);
            free(oldLocalStr);
            oldLocalStr = NULL;
        }
        
        // takes ownership over the input string. Might deallocate it !
        localStr = StrStandardizeScoringClient(localStr);

        // to not need to make NULL checks all the time, use empty string
        if (localStr == NULL)
        {
            localStr = (char*)malloc(1);
            if (localStr != NULL)
            {
                localStr[0] = 0;
            }
        }

        // prepare strings with anything that would help speed up string searches
        initStringDescriptorExtendedStruct(&scs->strOriginal, localStr);
        free(localStr);
        localStr = NULL;

        // searched string needs to be searched out of order in destination string
        char* SplitResults[MAX_SPLIT_RESULTS];
        if (scs->isQuoteEnclosed == 1)
        {
            scs->wordCount = 1;
            SplitResults[0] = strdup(scs->strOriginal.str);
        }
        else
        {
            StrSplit(scs->strOriginal.str, " ", (char**)&SplitResults, &scs->wordCount, MAX_SPLIT_RESULTS);
        }

        // create the store that we will return
        scs->wordDescriptors = (StringDescriptorExtendedStruct*)malloc(sizeof(StringDescriptorExtendedStruct) * scs->wordCount);

        // Allocation went wrong. Should not happen
        if (scs->wordDescriptors == NULL)
        {
            for (size_t i = 0; i < scs->wordCount; i++)
            {
                free(SplitResults[i]);
            }
            scs->wordCount = 0;
        }

        // store the list of strings as words
        for (size_t i = 0; i < scs->wordCount; i++)
        {
            initStringDescriptorExtendedStruct(&scs->wordDescriptors[i], SplitResults[i]);
            free(SplitResults[i]);
        }
    }
    else
    {
        memset(scs, 0, sizeof(SearchedString));
    }
}

void freeSearchedString(SearchedString* scs)
{
    if (scs == NULL)
    {
        return;
    }

    freeStringDescriptorExtendedStruct(&scs->strOriginal);

    if(scs->wordDescriptors != NULL)
    {
        for (size_t i = 0; i < scs->wordCount; i++)
        {
            freeStringDescriptorExtendedStruct(&scs->wordDescriptors[i]);
        }
        free(scs->wordDescriptors);
        scs->wordDescriptors = NULL;
        scs->wordCount = 0;
    }
}

void initDBString(DBString* scs, char* str)
{
    if (str != NULL)
    {
        size_t len = strlen(str);
        scs->str = (char*)malloc(len + 1);
        if (scs->str != NULL)
        {
            memcpy(scs->str, str, len + 1);
        }
        else
        {
            scs->str = strdup("");
        }
    }
    else
    {
        scs->str = strdup("");
    }
}

void freeDBString(DBString* scs)
{
    free(scs->str);
    scs->str = NULL;
}

/// <summary>
/// Compare 2 strings if they are equal. Strings are stored lower case + standardized
/// </summary>
/// <param name="scs1">String 1</param>
/// <param name="scs2">String 2</param>
/// <returns>1 if the searched string is found in DBstring</returns>
int IsSearchiStringInDbString(const SearchedString* searchedStr, const DBString* databaseStr)
{
//    printf("wordorder '%d', dbs '%s', searched s '%s', wordcount %d\n", keepWordOrder, databaseStr->str, searchedStr->strOriginal.str, searchedStr->wordCount);
    size_t wordsFound = 0;
    for (size_t index1 = 0; index1 < searchedStr->wordCount; index1++)
    {
        if( strstr(databaseStr->str, searchedStr->wordDescriptors[index1].str) )
        {
//          printf("found word %s in %s\n", searchedStr->wordDescriptors[index1].str, databaseStr->str);
            wordsFound++;
        }
        else
        {
//          printf("could not find word %s in %s\n", searchedStr->wordDescriptors[index1].str, databaseStr->str);
        }
    }

    // did we find all the words we were looking for ?
    return (wordsFound == searchedStr->wordCount);
}

/// <summary>
/// strdup a part of the input string. Adds terminator 0 to the end. Does not check out of bounds copy !
/// </summary>
/// <param name="source">input string</param>
/// <param name="start">character index where the copy should start</param>
/// <param name="charCount">Number of characters that will be copied from source</param>
/// <returns>substring or NULL on error</returns>
char* StrPartialStrDup(const char* source, const int start, const int charCount)
{
    if (source == NULL || start < 0 || charCount < 0)
    {
        return NULL;
    }

    char* ret = (char*)malloc(charCount + 1);
    if (ret == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < charCount; i++)
    {
        ret[i] = source[i + start];
    }

    //make sure the string is 0 terminated
    ret[charCount] = 0;

    return ret;
}

/// <summary>
/// Split the input string into the result store. The result will need to be destroyed
/// </summary>
/// <param name="source">Input string</param>
/// <param name="delim">Delimitator string</param>
/// <param name="res">array of character pointers. string will be allocated and placed as poitners</param>
/// <param name="resCount">number of substrings that are returned</param>
/// <param name="maxRes">Maximum number of substrings the "res" param can store</param>
/// <returns>error code</returns>
int StrSplit(const char* source, const char* delim, char** res, size_t* resCount, const size_t maxRes)
{
    if (source == NULL || delim == NULL || res == NULL || resCount == NULL || maxRes <= 0)
    {
        return 1;
    }

    const int delimSize = strlen(delim);
    const size_t str_size = strlen(source);
    size_t curPos = 0;

    *resCount = 0;
    do {
        // try to find the next delimiter
        const char* ptr = strstr(source + curPos, delim);
        // how many charactes do we need to copy from source ?
        size_t charCount;
        if (ptr != NULL)
        {
            charCount = ptr - source - curPos;
        }
        else
        {
            charCount = str_size - curPos;
        }
        // in case 2 delimiters came one after another, char count will be 0
        if (charCount > 0)
        {
            // strdup the partial string
            res[*resCount] = StrPartialStrDup(source, curPos, charCount);
            // increment number of results
            *resCount = *resCount + 1;
        }
        // jump after the delimiter we just found ( or outside the max string size )
        curPos += charCount + delimSize;
    } while (curPos < str_size || *resCount == maxRes);

    return 0;
}

/// <summary>
/// Check if the string is " enclosed
/// </summary>
/// <param name="str">Input string to check</param>
/// <returns>boolean value</returns>
static inline int StrIsQuoteEnclosed(const char* str)
{
    if (str == NULL)
    {
        return 0;
    }
    if (str[0] != '\"')
    {
        return 0;
    }
    const size_t len = strlen(str);
    if (len == 0 || str[len-1] != '\"')
    {
        return 0;
    }
    return 1;
}

/// <summary>
/// Convert string to lower case, in place
/// </summary>
/// <param name="str">Input string that will be updated</param>
inline void StrToLower(char* str)
{
    if (str == NULL)
    {
        return;
    }
    while (*str != '\0') 
    { 
        *str = (char)tolower(*str);
        str++;
    }
}

/// <summary>
/// Remove spaces from the start and the end of the string
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
void StrTrimScoringClient(char* str)
{
    if (str == NULL)
    {
        return;
    }
    size_t len = strlen(str);
    if(len==0)
    {
        return;
    }
    size_t SpacesBeggining = 0;
    ssize_t index = 0;
    while (str[index] == ' ' && index < len)
    {
        SpacesBeggining++;
        index++;
    
    }

    // move the remaining string to the beginning
    // also skips copying double ' '
    size_t writeIndex = 0;
    for (; index < len; index++)
    {
        // ignore multi spaces
        if (str[index] == ' ' && (str[index + 1] == ' ' || str[index + 1] == '\t'))
        {
            continue;
        }
        if (writeIndex != index)
        {
            str[writeIndex] = str[index];
        }
        writeIndex++;
    }
    str[writeIndex] = 0;
    len = writeIndex;

    // remove spaces from the end
    index = len - 1;
    while (index >= 0 && str[index] == ' ')
    {
        str[index] = 0;
        index--;
    }
}

/// <summary>
/// Check if a string is present at a specific position
/// </summary>
/// <param name="Longstr"></param>
/// <param name="substr"></param>
/// <returns></returns>
static inline int IsStrAt(const char* Longstr, const char* substr)
{
    return (strncmp(Longstr, substr, strlen(substr)) == 0);
}

/// <summary>
/// Sad replacement for GO language regexp
/// </summary>
/// <param name="c"></param>
/// <returns></returns>
static inline int IsWordCharacter(char c)
{
    return (isalnum(c) || c == '_');
}


#define ETV_NON_USED_VALUE -1 // this needs a whole system. Initially thought there are only 20 titles, turns out there can be millions
/// <summary>
/// Table to convert titles from short value / long value / int value
/// </summary>
struct EmployeTitleConvertParams
{
    const char* from;
    size_t fromLen;
    const char* to;
    size_t toLen;
    const char* to2;
    size_t to2Len;
    int title;
};

static int updateEmployeTitleConversionTable = 1;
// replace ' ' with underscore to not break up title into words. It will still be found if you look for partial words
static struct EmployeTitleConvertParams EmployeTitleConversionTable[] = {
    {.from = "vp", .to = "vice president", .to2 = "vice_president", .title = ETV_NON_USED_VALUE},
    {.from = "evp", .to = "executive vice president", .to2 = "executive_vice_president", .title = ETV_NON_USED_VALUE},
    {.from = "avp", .to = "associate vice president", .to2 = "associate_vice_president", .title = ETV_NON_USED_VALUE},
    {.from = "svp", .to = "senior vice president", .to2 = "senior_vice_president", .title = ETV_NON_USED_VALUE},
    {.from = "ceo", .to = "chief executive officer", .to2 = "chief_executive_officer", .title = ETV_NON_USED_VALUE},
    {.from = "cto", .to = "chief technology officer", .to2 = "chief_technology_officer", .title = ETV_NON_USED_VALUE},
    {.from = "cfo", .to = "chief financial officer", .to2 = "chief_financial_officer", .title = ETV_NON_USED_VALUE},
    {.from = "cmo", .to = "chief marketing officer", .to2 = "chief_marketing_officer", .title = ETV_NON_USED_VALUE},
    {.from = "coo", .to = "chief operating officer", .to2 = "chief_operating_officer", .title = ETV_NON_USED_VALUE},
    {.from = "sr", .to = "senior", .to2 = "senior", .title = ETV_NON_USED_VALUE},
    {.from = "jr", .to = "junior", .to2 = "junior", .title = ETV_NON_USED_VALUE},
    {.from = "ux", .to = "user experience", .to2 = "user_experience", .title = ETV_NON_USED_VALUE},
    {.from = "hr", .to = "human resources", .to2 = "human_resources", .title = ETV_NON_USED_VALUE},
    {.from = "r&d", .to = "research and development", .to2 = "research_and_development", .title = ETV_NON_USED_VALUE},
    {.from = "mgr", .to = "manager", .to2 = "manager", .title = ETV_NON_USED_VALUE},
    {.from = "dir", .to = "director", .to2 = "director", .title = ETV_NON_USED_VALUE},
    {.from = "qa", .to = "quality assurance", .to2 = "quality_assurance", .title = ETV_NON_USED_VALUE},
    {.from = "sde", .to = "software development engineer", .to2 = "software_development_engineer", .title = ETV_NON_USED_VALUE},
    {.from = "rn", .to = "registered nurse", .to2 = "registered_nurse", .title = ETV_NON_USED_VALUE},
    {.from = "cn", .to = "clinical nurse", .to2 = "clinical_nurse", .title = ETV_NON_USED_VALUE},
    {.from = NULL}
};

/// <summary>
/// Calculate the length of title strings so that memcmp could be used instead of strcmp
/// </summary>
void UpdateEmployeTitleConversionTable()
{
    // No point in calculating strlen countless times
    if (updateEmployeTitleConversionTable == 1)
    {
        updateEmployeTitleConversionTable = 0;

        size_t index = 0;
        while (EmployeTitleConversionTable[index].from != NULL)
        {
            EmployeTitleConversionTable[index].fromLen = strlen(EmployeTitleConversionTable[index].from);
            EmployeTitleConversionTable[index].to2Len = strlen(EmployeTitleConversionTable[index].to2);
            EmployeTitleConversionTable[index].toLen = strlen(EmployeTitleConversionTable[index].to);
            index++;
        }
    }
}

/// <summary>
/// Project specific function to extend abreviations to full values. This works inside a string where the title could be only part of the full string
/// </summary>
/// <param name="str"></param>
/// <returns>expanded string</returns>
char* StrStandardizeTitle(char* str)
{
    if (str == NULL)
    {
        return NULL;
    }

    char* res = str;

    // should run only once
    UpdateEmployeTitleConversionTable();

    size_t len = strlen(res);
    int WasWordCharacter = 0;
    for (size_t stri = 0; stri < len; stri++)
    {
        // if previous char was not a ' ', than we keep searching for a word start
        if (WasWordCharacter == 1)
        {
            WasWordCharacter = IsWordCharacter(res[stri]);
            continue;
        }
        WasWordCharacter = IsWordCharacter(res[stri]);

        // if a new word just started, check if it's a keyword
        if (WasWordCharacter == 1)
        {
            for (int i = 0; EmployeTitleConversionTable[i].from != NULL; i++)
            {
                // match the string at location
                size_t keywordlen = EmployeTitleConversionTable[i].fromLen;

                // will this keyword fit into the string ?
                if (stri + keywordlen <= len)
                {
                    // next character after the string is a non word character
                    int EndOfWord = !IsWordCharacter(res[stri + keywordlen]);
                    if (EndOfWord == 1)
                    {
                        // convert short format to long format
                        if (IsStrAt(&res[stri], EmployeTitleConversionTable[i].from) == 1)
                        {
                            // replace this substring with large string
                            size_t addedLen = EmployeTitleConversionTable[i].toLen;
                            size_t newStringSize = len - keywordlen + addedLen + 1;
                            char* newRes = (char*)malloc(newStringSize);

                            if (newRes == NULL)
                            {
                                return NULL;
                            }

                            strncpy(&newRes[0], res, stri);
                            strncpy(&newRes[stri], EmployeTitleConversionTable[i].to, addedLen);
                            strncpy(&newRes[stri + addedLen], &res[stri + keywordlen], len - stri - keywordlen + 1);

                            // ditch old string
                            free(res);

                            res = newRes;
                            len = strlen(res);
                            stri += addedLen;
                            WasWordCharacter = 0;
                            break;
                        }
                    }
                }

                // remove spaces from keywords to not break them into words
/*                keywordlen = EmployeTitleConversionTable[i].toLen;

                // will this keyword fit into the string ?
                if (stri + keywordlen <= len)
                {
                    // next character after the string is a non word character
                    int EndOfWord = !IsWordCharacter(res[stri + keywordlen]);
                    if (EndOfWord == 1)
                    {
                        // convert format with spaces to without spaces
                        if (IsStrAt(&res[stri], EmployeTitleConversionTable[i].from2) == 1)
                        {
                            memcpy(&res[stri], EmployeTitleConversionTable[i].to, EmployeTitleConversionTable[i].toLen);
                            break;
                        }
                    }
                }*/
            }
        }
    }

    return res;
}
/// <summary>
/// Create a new string without punctuation
/// There are pro and against arguments for removing characters from a string
/// </summary>
/// <param name="str"></param>
/// <returns></returns>
void StrRemovePunctuation(char* str)
{
    if (str == NULL)
    {
        return;
    }
    const static char punctuations[] = { '.',',','/','(',')',':','-','\n','\t','\r' };
    int len = strlen(str);    
    size_t writeIndex = 0;
    for (size_t readIndex = 0; readIndex < len; readIndex++)
    {
        size_t foundPunctuation = 0;
        for (int i = 0; i < sizeof(punctuations); i++)
        {
            if (str[readIndex] == punctuations[i])
            {
                foundPunctuation = 1;
                break;
            }
        }
        if (foundPunctuation == 1)
        {
            // replace any punctuation with a ' ' to avoid merging strings
            // are there any situations where this is needed ? Chances are we create strings that did not exist before
//            str[writeIndex] = ' '; 
            // Xuan said since we already have data that removes punctuations, there is no other option but to remove them in the future also
            continue;
        }
        else if (readIndex != writeIndex)
        {
            str[writeIndex] = str[readIndex];
        }
        writeIndex++;
    }
    str[writeIndex] = 0; // in case the string got shorter, make sure we terminate it earlier than before

    return;
}

char* StrStandardizeScoringClient(char* str)
{
    char* localStr = str;

    // inplace lowercase the string. Needs to happen before standardization
    StrToLower(localStr);

    // only some of the fields require this, but it is probably best to do it for every string
    StrRemovePunctuation(localStr);

    // remove ' ' from beginning and ending. Also removes consecutive ' ' chars
    StrTrimScoringClient(localStr);

    // exampand abreviations. Might reallocate(free+alloc) the input string
    localStr = StrStandardizeTitle(localStr);

    return localStr;
}

/// <summary>
/// strncpy has a SSE implementation that can not copy pointers with same source and destination
/// </summary>
/// <param name="dst"></param>
/// <param name="src"></param>
static inline void moveStringToLeft(char* dst, char* src)
{
    if (dst == src)
    {
        return;
    }
    while (*src != 0)
    {
        *dst = *src;
        dst++;
        src++;
    }
    *dst = 0;
}

int StrReplaceAllToSmaller(char* largeStr, const char* substr, const char* replace)
{
    size_t replaceLen = strlen(replace);
    size_t substrLen = strlen(substr);
    if (substrLen == 0 || replaceLen > substrLen)
    {
        return -1;
    }
    char* startLoc;
    int changesMade = 0;
    while ((startLoc = strstr(largeStr, substr)))
    {
        if (replaceLen == 0)
        {
            moveStringToLeft(startLoc, &startLoc[substrLen]);
        }
        else
        {
            // replace substr with 'replace'
            memcpy(startLoc, replace, replaceLen);
            // move the remaining part of the string to the left
            moveStringToLeft(&startLoc[replaceLen], &startLoc[substrLen]);
        }
        changesMade++;
    }
    return changesMade;
}