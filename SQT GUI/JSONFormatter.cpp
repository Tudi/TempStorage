#include "StdAfx.h"
#include "ResponseFormatter.h"
#include "OQAPI.h"
#include "JSONFormatter.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

/*
    This should be rewritten using resource files. Would add language support also
    Or you could pack every ID + string + handler + ... into a "class" or struct ....
*/

// list of ID -> Name lookups
char *ResponseLookup[VT_COUNT];
// list of value to JSON writer functions for each value type
typedef void (*JSONValueWriterFunc)(DataStore *, int, ValStore *, char *, int );
JSONValueWriterFunc ValueHandlers[VT_COUNT];
// Should be used as constructor to build packet handlers ( readers / writers )
int IsLookupInitialized = 0;

void BuildLookupStrings()
{
    if( IsLookupInitialized == 1 )
        return;

    memset( ResponseLookup, 0, sizeof( ResponseLookup ) );
    memset( ValueHandlers, 0, sizeof( ValueHandlers ) );

    //there should be no gaps
    ResponseLookup[VT_CAN_DO_IQ] = "CanDoIQ";
    ResponseLookup[VT_CAN_DO_OQ] = "CanDoOQ";
    ResponseLookup[VT_CAN_DO_OQMVM] = "CanDoOQMVM";
    ResponseLookup[VT_NODE_LIST_COUNT] = "NodeNameList";
    ResponseLookup[VT_NODE_LIST_NAME] = "NodeName";
    ResponseLookup[VT_NODE_PERFORM_NAME] = "NodeNameForSQT";
    ResponseLookup[VT_NODE_PERFORM_IQ] = "PerformIQ";
    ResponseLookup[VT_NODE_PERFORM_OQ] = "PerformOQ";
    ResponseLookup[VT_NODE_PERFORM_OQMVM] = "PerformOQMVM";
    ResponseLookup[VT_ZIP_COMPRESSED_PACKET] = "CompressionType";
    ResponseLookup[VT_SSL_ENCRYPTED] = "EncryptionType";
    ResponseLookup[VT_SESSION_ID] = "SessionID";
    ResponseLookup[VT_ERROR_ID] = "ErrorID";
    ResponseLookup[VT_ERROR_STRING] = "Error";

    //this is just a safety check for the sake of debugging
    for( int i = VT_FIRST_UNUSED_TYPE + 1; i < VT_COUNT; i++ )
        if( ResponseLookup[i] == NULL )
            printf("Warning. Variable type %d has no name assigned. Is this acceptable ?\n", i );

    ValueHandlers[VT_NODE_LIST_COUNT]=&WriteUnIndexedNameVector;
    ValueHandlers[VT_NODE_LIST_NAME]=&WriteNoValue;

    ValueHandlers[VT_ERROR_ID]=&WriteUnIndexedErrorWithString;
    ValueHandlers[VT_ERROR_STRING]=&WriteNoValue;

    IsLookupInitialized = 1;
}

char *ValStoreGetVariableName(int Type)
{
    if( Type >= VT_COUNT || ResponseLookup[ Type ] == NULL )
        return "";
    return ResponseLookup[ Type ];
}

void ValueToJSON( DataStore *DS, int at, ValStore *V, char *Out, int MaxLen )
{
    // y u do thiz Dolan ?
    if( Out == NULL )
        return;
    // safety. Just in case ...
    strcpy_s( Out, MaxLen, "" );
    // you might need to update ...
    if( V->Type >= VT_COUNT || V->Type <= 0 )
        return;
    //one time initialization. Should move this to a constructor
    BuildLookupStrings();

    //if we have a special handler for this type of value ( structures ? )
    if( ValueHandlers[ V->Type ] != NULL )
    {
        ValueHandlers[ V->Type ]( DS, at, V, Out, MaxLen );
        return;
    }

    // string handling
    if( V->Type == VT_ERROR_STRING || V->Type == VT_NODE_LIST_NAME )
        sprintf_s( Out, MaxLen, "\"%s\":\"%s\"", ValStoreGetVariableName( V->Type ), DS_GetStringValue( V ) );
    // generic int handling
    else if( V->Len == 1 )
        sprintf_s( Out, MaxLen, "\"%s\":%d", ValStoreGetVariableName( V->Type ), DS_GetIntValue( V ) & 0xFF );
    else if( V->Len == 2 )
        sprintf_s( Out, MaxLen, "\"%s\":%d", ValStoreGetVariableName( V->Type ), DS_GetIntValue( V ) & 0xFFFF );
    else if( V->Len == 3 )
        sprintf_s( Out, MaxLen, "\"%s\":%d", ValStoreGetVariableName( V->Type ), DS_GetIntValue( V ) & 0xFFFFFF );
    else if( V->Len == 4 )
        sprintf_s( Out, MaxLen, "\"%s\":%d", ValStoreGetVariableName( V->Type ), DS_GetIntValue( V ) & 0xFFFFFF );
}

/*
    Right now it does not support sub structure parsing. Maybe some other time
*/
char *ResponseToString( DataStore *DS )
{
    // easy
    if( DS == NULL )
        return NULL;

    //one time initialization. This shouldbe moved to a constructor
    BuildLookupStrings();

    //default value we can work with
    int NewSize = 3;
    char *Response = (char *)malloc( NewSize );
    strcpy_s( Response, NewSize, "{" );

    //parse the DS and translate it into a JSON
    int NodeCount = DS->Count;
    int ValuesWritten = 0;
    for( int i = 0; i < NodeCount; i++ )
    {
        char AsString[ 32000 ];  // hmmm, this could get too small
        ValStore  *Val = DS_GetValue( DS, i );
        // buget
        if( Val == NULL )
            continue;
        //convert to JSON
        ValueToJSON( DS, i, Val, AsString, sizeof( AsString ) );
        //add it to response list if there is a value to write
        if( AsString[ 0 ] == 0 )
            continue;
        //add it to the JSON string
        NewSize = strlen( Response ) + strlen( AsString ) + 3;
        Response = (char*)realloc( Response, NewSize );
        sprintf_s( Response, NewSize, "%s%s,", Response, AsString );
        ValuesWritten++;
    }

    // close JSON list
    if( ValuesWritten == 0 )
        strcpy_s( Response, NewSize, "" );
    else if( ValuesWritten > 0 )
    {
        //pinch off last character
        Response[ strlen( Response ) - 1 ] = 0;
        //write vector ending
        sprintf_s( Response, NewSize, "%s}", Response );
    }

    return Response;
}

void PrintResponseAsJSON( DataStore *in )
{
    if( in == NULL )
        return;
    char *JSONresp = ResponseToString( in );
    if( JSONresp )
    {
        printf("response as JSON : %s \n", JSONresp );
        Dispose_JSON( &JSONresp );
    }
    else
        printf("Could not format DS into JSON\n");
}

void Dispose_JSON( char **Buf )
{
    if( Buf == NULL )
        return;
    if( *Buf != NULL )
        free( *Buf );
    *Buf = NULL;
}